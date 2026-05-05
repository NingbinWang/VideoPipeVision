#include "MediaStream.h"
#include "New.h"
#include "SysMemory.h"
#include "Thread.h"
#include "SysTime.h"
#include "MediaStream.h"
#include "MediaPriv.h"
#include "MediaDecInner.h"
#include "SysMutex.h"


#define MAX_VIDEO_ELEMENTARY_LEN    (0x500000)
#define MAX_AUDIO_ELEMENTARY_LEN    (0x1000)
#define MAX_PRIVT_ELEMENTARY_LEN    (64*1024)
#define RTP_STREAM_LEN           (1400)
#define MAX_PES_PACKET_LEN       (65496)
#define MAX_VIDEO_BUFFSIZE          (0x400000)



MediaStream* MediaStream::createNew(MEDIA_PARAM_T& Param)
{
    return New<MediaStream>::allocate(&Param);
}


MediaStream::MediaStream(MEDIA_PARAM_T *pParam):
mpParam(pParam)
{
	//mpSendStreamToRecPoolMutex = Mutex::createNew();
	//mpSendStreamToDecPoolMutex = Mutex::createNew();
	//mpSendStreamToEncPoolMutex = Mutex::createNew();
}

MediaStream::~MediaStream() 
{
	
}

INT32  MediaStream::StreamPack(MEDIA_ENC_FRAME_T* pFrame)
{
	UINT uIndex = 0;
	//MEDIA_ELEMENTARY_STREAM_T stEleStream;
	MEDIA_STREAM_PACK_PARAM_T stParam;
	memset((void *)&stParam, 0, sizeof(MEDIA_STREAM_PACK_PARAM_T));
	//memset((void *)&stEleStream, 0, sizeof(MEDIA_ELEMENTARY_STREAM_T));
	stParam.uNaluNum = pFrame->u32NaluNum;
	for(uIndex = 0;uIndex < stParam.uNaluNum;uIndex++)
	{
		stParam.astNalu[uIndex].u32NaluLen = pFrame->astNalu[uIndex].u32NaluLen;
		stParam.astNalu[uIndex].u32NaluType = pFrame->astNalu[uIndex].u32NaluType;
		stParam.astNalu[uIndex].pNaluPtr = pFrame->astNalu[uIndex].pNaluPtr;
	}
	return OK;
}

INT32  MediaStream::StreamFrameConvert(MEDIA_VIDEO_FRAME_T* pFrameInfo,UINT uVoChan)
{
	IMAGE_FRAME_T srcimg = {0};
	IMAGE_FRAME_T dstimg = {0};
	UINT32 uFrameSize = 0;
	CHAR* pPackOutBuf = (CHAR*)SysMemory_malloc(MAX_VIDEO_ELEMENTARY_LEN);
	CHAR *pOutputbuf = (CHAR*)SysMemory_malloc(MAX_VIDEO_BUFFSIZE);
	CHAR *pFramebuf = (CHAR*)SysMemory_malloc(MAX_VIDEO_BUFFSIZE);
	MEDIA_VIDEO_FRAME_T stDecFrameInfo;
	memset((void *)&stDecFrameInfo, 0, sizeof(MEDIA_VIDEO_FRAME_T));
	memset((void *)pPackOutBuf,0,MAX_VIDEO_ELEMENTARY_LEN);
	memset((void *)pOutputbuf,0,MAX_VIDEO_BUFFSIZE);
	srcimg.width =  pFrameInfo->stVideoHeader.u32ImageWidth;
    srcimg.height = pFrameInfo->stVideoHeader.u32ImageHeight;
    srcimg.width_stride = pFrameInfo->stVideoHeader.u32ImageWidth;
    srcimg.height_stride = pFrameInfo->stVideoHeader.u32ImageHeight;
	SysMemory_copy((VOID *)pFramebuf, (VOID *)pFrameInfo->stImageFrame.pVirAddr, pFrameInfo->stImageFrame.sSize);
    srcimg.virt_addr = pFramebuf;
    srcimg.format = MediaRgaFmtTranslation(pFrameInfo->stVideoHeader.eFormatType);
	dstimg.width =  (int)mpParam->astVoCfgParam[uVoChan].u32ImageWidth;
    dstimg.height = (int)mpParam->astVoCfgParam[uVoChan].u32ImageHeight;
    dstimg.width_stride = (int)mpParam->astVoCfgParam[uVoChan].u32ImageWidth;
    dstimg.height_stride = (int)mpParam->astVoCfgParam[uVoChan].u32ImageHeight;
    dstimg.virt_addr = (CHAR*)pOutputbuf;
    dstimg.format =  MediaRgaFmtTranslation(mpParam->astVoCfgParam[uVoChan].eType);
	MediaDecCvtcoloerVirt(&srcimg,&dstimg);
	uFrameSize =  mpParam->astVoCfgParam[uVoChan].u32FrameSize;
	stDecFrameInfo.stImageFrame.sSize = uFrameSize;
	stDecFrameInfo.stVideoHeader.eFormatType = mpParam->astVoCfgParam[uVoChan].eType;
	stDecFrameInfo.stVideoHeader.iframeNum = pFrameInfo->stVideoHeader.iframeNum;
	stDecFrameInfo.stVideoHeader.u32ImageHeight = mpParam->astVoCfgParam[uVoChan].u32ImageHeight;
	stDecFrameInfo.stVideoHeader.u32ImageWidth = mpParam->astVoCfgParam[uVoChan].u32ImageWidth;
	stDecFrameInfo.stVideoHeader.iHeightStride = dstimg.height_stride;
	stDecFrameInfo.stVideoHeader.iWidthStride = dstimg.width_stride;
	//stDecFrameInfo.stImageFrame.pVirAddr = (VOID *)(pPackOutBuf+sizeof(MEDIA_VIDEO_FRAME_T));
	SysMemory_copy((VOID *)pPackOutBuf, &stDecFrameInfo,sizeof(MEDIA_VIDEO_FRAME_T));
	SysMemory_copy((VOID *)(pPackOutBuf+sizeof(MEDIA_VIDEO_FRAME_T)),pOutputbuf,uFrameSize);
	SendStreamToDecPool(mpParam->astVoCfgParam[uVoChan].uDecChan,(PUINT8)pPackOutBuf,uFrameSize+sizeof(MEDIA_VIDEO_FRAME_T),0);
	SysMemory_free(pPackOutBuf);
	SysMemory_free(pOutputbuf);
	SysMemory_free(pFramebuf);
	return OK;
}

INT32  MediaStream::SendStreamToDecPool(UINT32 uChan,PUINT8 pStreamSrc,UINT32 uLength,INT32 iDrop)
{
	UINT32 u32W = 0;
	UINT32 u32R = 0;
	UINT32 u32SpareLen = 0;
	UINT32 u32Part1 = 0;
	UINT32 u32Part2 = 0;
	UINT32 u32TotalLen = 0;
	PUINT8 ptr = NULL;
    DEC_SHARE_BUF_T* pPool = &(mpParam->aDecPool[uChan]);
	if(pStreamSrc == NULL)
	{
		LOG_ERROR("pStreamSrc is NULL\n");
		return ERROR;
	}
	if((void *)pPool->addr[0] == NULL)
	{
		LOG_ERROR("addr is NULL\n");
		return ERROR;
	}
	//uLength = (uLength + 3) & ~3; // 将 uLength 向上舍入到最小的、不小于它的 4 的倍数，常用于4 字节对齐
	if(pPool->totalLen == 0){
		LOG_ERROR("totalLen is NULL\n");
		return ERROR;
	}
	MUTEX_ID* pMutex = &pPool->mDevPool;
	SysMutex_lock(pMutex,WAIT_FOREVER);
	u32W = pPool->wIdx;	//读写指针获得锁之后再赋值，避免不同线程的读写指针可能相同
	if(iDrop)
	{
		u32R = pPool->wIdx;
	}else{
		u32R = pPool->rIdx;
	}
	u32TotalLen = mpParam->aDecPool[uChan].totalLen;
	ptr = (PUINT8)mpParam->aDecPool[uChan].addr[0];
	u32SpareLen = (u32R + u32TotalLen - u32W - 1) % u32TotalLen;
	
	if(uLength > u32SpareLen)
	{
		LOG_INFO("Lost one frame to dec pool,len=%d,spareLen=%d rIdx=%d  wIdx=%d \n",uLength,u32SpareLen,u32R,u32W);
		SysMutex_unlock(pMutex);
		return OK;
	}
	u32Part1 = u32TotalLen - u32W;
	if(uLength > u32Part1)
	{
		
		SysMemory_copy((void *)(ptr + u32W),(void *)pStreamSrc, u32Part1);
		u32Part2 = uLength - u32Part1;
		SysMemory_copy((void *)ptr,(void *)(pStreamSrc+u32Part1), u32Part2);
	}
	else
	{
		SysMemory_copy((void *)(ptr+u32W),(void *)pStreamSrc,uLength);
	}	
	pPool->wIdx = (pPool->wIdx + uLength) % (UINT)u32TotalLen;
	SysMutex_unlock(pMutex);
	return OK;
}

INT32  MediaStream::SendStreamToEncPool(UINT32 uChan,PUINT8 pStreamSrc,UINT32 uLength,INT32 iDrop)
{
	UINT32 u32W = 0;
	UINT32 u32R = 0;
	UINT32 u32SpareLen = 0;
	UINT32 u32Part1 = 0;
	UINT32 u32Part2;
	UINT32 u32TotalLen;
	PUINT8 ptr = NULL;
    ENC_SHARE_BUF_T* pPool = &(mpParam->aEncPool[uChan]);
	if(pStreamSrc == NULL)
	{
		LOG_ERROR("pStreamSrc is NULL\n");
		return ERROR;
	}
	if((void *)pPool->addr[0] == NULL)
	{
		LOG_ERROR("addr is NULL\n");
		return ERROR;
	}
	if(pPool->totalLen == 0){
		LOG_ERROR("totalLen is NULL\n");
		return ERROR;
	}
	MUTEX_ID* pMutex = &pPool->mEncPool;
	SysMutex_lock(pMutex,WAIT_FOREVER);
	u32W = pPool->wIdx; //读写指针获得锁之后再赋值，避免不同线程的读写指针可能相同
	if(iDrop)
	{
			u32R = pPool->wIdx;
	}else{
			u32R = pPool->rIdx;
	}

	u32TotalLen = mpParam->aEncPool[uChan].totalLen;
	ptr = (PUINT8)mpParam->aEncPool[uChan].addr[0];
	u32SpareLen = (u32R + u32TotalLen - u32W - 1) % u32TotalLen;	
	if(uLength > u32SpareLen)
	{
		LOG_INFO("Lost one frame to enc pool,len=%d,spareLen=%d rIdx=%d  wIdx=%d \n",uLength,u32SpareLen,u32R,u32W);
		SysMutex_unlock(pMutex);
		return OK;
	}
	u32Part1 = u32TotalLen - u32W;
	if(uLength > u32Part1)
	{
		u32Part2 = uLength - u32Part1;
		SysMemory_copy((void *)(ptr + u32W),(void *)pStreamSrc, u32Part1);
		SysMemory_copy((void *)ptr,(void *)(pStreamSrc+u32Part1), u32Part2);
	}
	else
	{
		SysMemory_copy((void *)(ptr+u32W),(void *)pStreamSrc,uLength);
	}	
	pPool->wIdx = (pPool->wIdx + uLength) % (UINT)u32TotalLen;
	SysMutex_unlock(pMutex);
	return OK;
}

INT32  MediaStream::SendStreamToAudioPool(PUINT8 pStreamSrc,UINT32 uLength,INT32 iDrop)
{
	UINT32 u32W = 0;
	UINT32 u32R = 0;
	UINT32 u32SpareLen = 0;
	UINT32 u32Part1 = 0;
	UINT32 u32Part2;
	UINT32 u32TotalLen;
	PUINT8 ptr = NULL;
    AUDIO_POOL_INFO_T* pPool = &(mpParam->stAudioPool);
	if(pStreamSrc == NULL)
	{
		LOG_ERROR("pStreamSrc is NULL\n");
		return ERROR;
	}
	if((void *)pPool->addr[0] == NULL)
	{
		LOG_ERROR("addr is NULL\n");
		return ERROR;
	}
	if(pPool->totalLen == 0){
		LOG_ERROR("totalLen is NULL\n");
		return ERROR;
	}
	MUTEX_ID* pMutex = &pPool->mAudPool;
	SysMutex_lock(pMutex,WAIT_FOREVER);
	u32W = pPool->wIdx; //读写指针获得锁之后再赋值，避免不同线程的读写指针可能相同
	if(iDrop)
	{
			u32R = pPool->wIdx;
	}else{
			u32R = pPool->rIdx;
	}

	u32TotalLen = pPool->totalLen;
	ptr = (PUINT8)pPool->addr[0];
	u32SpareLen = (u32R + u32TotalLen - u32W - 1) % u32TotalLen;	
	if(uLength > u32SpareLen)
	{
		LOG_INFO("Lost one frame to enc pool,len=%d,spareLen=%d rIdx=%d  wIdx=%d \n",uLength,u32SpareLen,u32R,u32W);
		SysMutex_unlock(pMutex);
		return OK;
	}
	u32Part1 = u32TotalLen - u32W;
	if(uLength > u32Part1)
	{
		u32Part2 = uLength - u32Part1;
		SysMemory_copy((void *)(ptr + u32W),(void *)pStreamSrc, u32Part1);
		SysMemory_copy((void *)ptr,(void *)(pStreamSrc+u32Part1), u32Part2);
	}
	else
	{
		SysMemory_copy((void *)(ptr+u32W),(void *)pStreamSrc,uLength);
	}	
	pPool->wIdx = (pPool->wIdx + uLength) % (UINT)u32TotalLen;
	SysMutex_unlock(pMutex);
	return OK;
}


INT32  MediaStream::SendStreamToRecPool(UINT32 uChan,PUINT8 pStreamSrc,UINT32 uLength, BOOL bVideo, BOOL dropMode)
{
	UINT32 u32W = 0;
	UINT32 u32R = 0;
	UINT32 u32SpareLen = 0;
	UINT32 u32Part1 = 0;
	UINT32 u32Part2;
	UINT32 u32TotalLen;
	PUINT8 ptr;
    REC_POOL_INFO_T *pRecPool;
    SYS_DATE_TIME_T  stNowTime = {0};
    pRecPool=&mpParam->astRecPool[uChan];
    
	if((NULL == pStreamSrc) || (0 == uLength) || (0 == pRecPool->totalLen))
    {
        LOG_ERROR("pStreamSrc= %p len=0x%x  pRecPool->totalLen=0x%x!\n",pStreamSrc,uLength,pRecPool->totalLen);
        return ERROR;
    }
    
    if((void*)pRecPool->addr[0] == NULL)
    {
        LOG_ERROR("pAddr is NULL \n");
        return ERROR;
    }
    
	uLength = (uLength + 3) & ~3;//4字节对齐
    //MutexLockGuard mutexLockGuard(mpSendStreamToRecPoolMutex);
    if (pRecPool->totalLen)
    {
        u32W = pRecPool->wIdx;
        if(!dropMode)
        {
            u32R = pRecPool->rIdx; //判断读指针，不循环覆盖
        }
        else
        {
            u32R = pRecPool->wIdx; //不判断读指针，循环覆盖
        }
        u32TotalLen = pRecPool->totalLen;
        ptr = (PUINT8)pRecPool->addr[0];
        u32SpareLen = (u32R + u32TotalLen - u32W - 1) % u32TotalLen;

        //统计上传到录像缓冲区次数
        if(bVideo)
        {
            pRecPool->vFrmCounter++;
        }
        
        if(uLength > u32SpareLen)
        {
            //mpParam->encStatus[chan].RecPoolFrmLost++;
			LOG_WARNING("chan[%d] RecPool is overflow w=%d r=%d len=%d totalLen=%d!\n",uChan,u32W,u32R,uLength,u32TotalLen);
            return ERROR;
        }
        
        u32Part1 = u32TotalLen - u32W;

        if(uLength > u32Part1)
        {
            memcpy((PUINT8)(ptr + u32W),(PUINT8)pStreamSrc,  u32Part1);
            u32Part2 = uLength - u32Part1;
            memcpy((PUINT8)ptr, (PUINT8)(pStreamSrc+u32Part1), u32Part2);
        }
        else
        {
            memcpy((PUINT8)(ptr+u32W),(PUINT8)pStreamSrc,  uLength);
        }
        SysTime_get_msec(&pRecPool->lastFrameStdTime);
        SysTime_get_in_struct(&stNowTime);
        memcpy((void *)&pRecPool->lastFrameAbsTime,(void *)&stNowTime,sizeof(DATE_TIME_T));
        pRecPool->wIdx = (pRecPool->wIdx + uLength) % u32TotalLen;
        LOG_DEBUG("[DSP] chan[%d]SendStreamToRecPool w=%d, r=%d len=%d\n",uChan,pRecPool->wIdx,pRecPool->rIdx,uLength);
    }
        
    return OK;
}










