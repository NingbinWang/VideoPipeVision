
#include "MediaManager.h"
#include "SysMemory.h"
#include "autoconf.h"
#include "Logger.h"
#include "SysTime.h"
#include "SysMutex.h"
#include <alsa/asoundlib.h>

MEDIA_PARAM_T *pManagerParm = NULL;

int MediaManagerInit()
{
	UINT32 shareTotalSize = 0;
	UINT32 uIndex = 0;
	pManagerParm = (MEDIA_PARAM_T*)SysMemory_malloc(sizeof(MEDIA_PARAM_T));
	memset((void *)pManagerParm,0,sizeof(MEDIA_PARAM_T));
	//VI
	pManagerParm->u32ViChanCnt = 1;
	pManagerParm->astViCfgParam[0].u32Enable = TRUE;
	pManagerParm->astViCfgParam[0].u32Chn = 0;
	pManagerParm->astViCfgParam[0].eViType = VI_V4L2;
	pManagerParm->astViCfgParam[0].u32Image_viW = 1920;
	pManagerParm->astViCfgParam[0].u32Image_viH = 1080;
	pManagerParm->astViCfgParam[0].u32Frame_rate = 30;
	pManagerParm->astViCfgParam[0].eSensorType = CMOS_OV_5969;
	pManagerParm->astViCfgParam[0].u32Frame_rate = 30;

	strcpy(pManagerParm->astViCfgParam[0].strDevname,MAINDEVNAME);
    if(pManagerParm->astViCfgParam[0].eViType == VI_V4L2)
    {
    	if(strlen(MAINFORMAT) != 4){
			LOG_ERROR("set in formate %s is not right length!\n",MAINFORMAT);
    	}
    }
	strcpy(pManagerParm->astViCfgParam[0].strFormat,MAINFORMAT);
	for(uIndex = 0;uIndex < pManagerParm->u32ViChanCnt;uIndex++){
			pManagerParm->astViCfgParam[uIndex].eViewMirror = VIEW_NATURAL;
			pManagerParm->astViCfgParam[uIndex].stDayNightInfo.u32DayNightMode = 0;
			pManagerParm->astViCfgParam[uIndex].u8EnWdr = 1;
			pManagerParm->astViCfgParam[uIndex].u8WdrLevel = 50;
	}
	//enc
	pManagerParm->u32EncChanCnt = 1;
	pManagerParm->astEncCfgParam[0].uChan = 0;
	pManagerParm->astEncCfgParam[0].uEncW = 1920;
	pManagerParm->astEncCfgParam[0].uEncH = 1080;
	pManagerParm->astEncCfgParam[0].eEncoderType = MEDIA_ENC_CODETYPE_AVC;
	strcpy(pManagerParm->astEncCfgParam[0].strStreamType,MAINFORMAT);
	for(uIndex = 0;uIndex < pManagerParm->u32EncChanCnt;uIndex++){
		SysMutex_create(&pManagerParm->aEncPool[uIndex].mEncPool, 0);
		pManagerParm->aEncPool[uIndex].rIdx = 0;
		pManagerParm->aEncPool[uIndex].wIdx = 0;
		pManagerParm->aEncPool[uIndex].totalLen = 12*1024*1024;
		pManagerParm->aEncPool[uIndex].addr[0] = (PUINT8)SysMemory_malloc(pManagerParm->aEncPool[uIndex].totalLen);
    }
	//encoder status
	pManagerParm->stEncoderStatus.stEncBasicParam.bEnable = true;
	pManagerParm->stEncoderStatus.stEncBasicParam.encH = 1080;
	pManagerParm->stEncoderStatus.stEncBasicParam.encW = 1920;
	//vo
	pManagerParm->u32VoChanCnt = 1;
	pManagerParm->astVoCfgParam[0].uChan = 0;
	pManagerParm->astVoCfgParam[0].eType = MEDIA_FORMAT_BGRA8888;
	pManagerParm->astVoCfgParam[0].uDecChan = 0;
	pManagerParm->astVoCfgParam[0].u32ImageHeight = 1080;
	pManagerParm->astVoCfgParam[0].u32ImageWidth = 720;
	pManagerParm->astVoCfgParam[0].u32FrameSize = 1080*720*4;//一帧的数据是 高*宽*位宽
	//OSD
	pManagerParm->bOSD = true;
	//音频
	pManagerParm->stAudioCfgParam.uChan = 0;
	pManagerParm->stAudioCfgParam.uPcmChannel = 2;
	pManagerParm->stAudioCfgParam.uPcmSampleRate = 48000;
	pManagerParm->stAudioCfgParam.uPcmAccess = SND_PCM_ACCESS_RW_INTERLEAVED;//SND_PCM_STREAM_PLAYBACK 0  SND_PCM_STREAM_CAPTURE 1
	pManagerParm->stAudioCfgParam.uPcmFormat = SND_PCM_FORMAT_S16_LE;//SND_PCM_FORMAT_S16_LE 2
	pManagerParm->stAudioCfgParam.uPcmFrameSize = 1024;
	pManagerParm->stAudioCfgParam.stCodecParam.eType = AUCODEC_ACC;
	pManagerParm->stAudioCfgParam.stCodecParam.uBitRate = 128000;
	pManagerParm->stAudioCfgParam.stCodecParam.uTransMux = 0;
	pManagerParm->stAudioCfgParam.stCodecParam.uAOT = 2;

	
	strcpy(pManagerParm->stAudioCfgParam.strDevname,AUDIODEVNAME);
	SysMutex_create(&pManagerParm->stAudioPool.mAudPool, 0);
	pManagerParm->stAudioPool.totalLen = 256*1024;
	pManagerParm->stAudioPool.rIdx = 0;
	pManagerParm->stAudioPool.wIdx = 0;
	//音频的内存池
	pManagerParm->stAudioPool.addr[0] = SysMemory_malloc(pManagerParm->stAudioPool.totalLen);
    //RecPool池
	pManagerParm->astRecPool[0].totalLen = 12*1024*1024;
	pManagerParm->astRecPool[1].totalLen = 4*1024*1024;//不使用
	pManagerParm->astRecPool[2].totalLen = 4*1024*1024;//不使用
	pManagerParm->astRecPool[0].addr[0] = (PUINT8)SysMemory_malloc(pManagerParm->astRecPool[0].totalLen);
	//voPool池
	pManagerParm->u32DecChanCnt = 1;
	for(uIndex = 0;uIndex < pManagerParm->u32DecChanCnt;uIndex++){
			//VO数据流Pool池
			SysMutex_create(&pManagerParm->aDecPool[uIndex].mDevPool, 0);
			pManagerParm->aDecPool[uIndex].rIdx = 0;
			pManagerParm->aDecPool[uIndex].wIdx = 0;
			pManagerParm->aDecPool[uIndex].totalLen = 12*1024*1024;
			pManagerParm->aDecPool[uIndex].addr[0] = (PUINT8)SysMemory_malloc(pManagerParm->aDecPool[uIndex].totalLen);
	}
	MediaInit(pManagerParm);
	return 0;
}

MEDIA_PARAM_T * MediaManagerGet()
{
	return pManagerParm;
}

unsigned int  GetDecStream(unsigned int uChan,void *pUserData,INT32 iDrop)
{
   
    UINT uLen1 = 0;
	UINT uLen2 = 0;
	UINT uCurLen=0;
    VOID * pData = NULL;
    UINT uR = 0;
	UINT uW = 0;
	UINT uFrameSize = 0;
    DEC_SHARE_BUF_T *pPool = NULL;
	MEDIA_VIDEO_FRAME_T* pFrame;
    bool bHaveData = true;
	MUTEX_ID* pMutex = NULL;
	if(pManagerParm == NULL)
	{
		return 0;
	}
	if((void *)pManagerParm->aDecPool[uChan].addr[0] == NULL)
	{
		return 0;
	}
	pMutex = &(pManagerParm->aDecPool[uChan].mDevPool);
	SysMutex_lock(pMutex,WAIT_FOREVER);
	uFrameSize = sizeof(MEDIA_VIDEO_FRAME_T)+pManagerParm->astVoCfgParam[0].u32FrameSize;
    while (bHaveData)
    {
        bHaveData = false;
        pPool=&(pManagerParm->aDecPool[uChan]);
		if(pPool == NULL)
		{
		   LOG_ERROR("NO RAWPOOL!\n");
		   return 0;
		}
        uW=pPool->wIdx;
		if(!iDrop)
		{
			 uR=pPool->rIdx;
		}else{
			 uR=pPool->wIdx;
		}
        // get data length in share memory
        if (uW >= uR)
        {
                uLen1 = uW - uR;
                uLen2 = 0;
         }
         else
         {
                uLen1 = pPool->totalLen - uR;
                uLen2 = uW;
         }
         uCurLen = uLen1+uLen2;
         if(uCurLen < uFrameSize)
         {
               LOG_INFO("uCurLen too small uCurLen = %d uFrameSize = %d\n",uCurLen,uFrameSize);
         	   SysMutex_unlock(pMutex);
               return 0;
         }
         if(uCurLen >= uFrameSize)
         {          
                if((void*)pPool->addr[0] == NULL)
                {
                    LOG_ERROR("[chan%d] invalid addr[0] !\n",uChan);
					SysMutex_unlock(pMutex);
                    return 0;
                }
				pData = SysMemory_malloc(uFrameSize);
				if(uLen1 < uFrameSize){
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uLen1);
					SysMemory_copy((void*)((PUINT8)pData+uLen1),(void *)(PUINT8)pPool->addr[0], uFrameSize-uLen1);
				}else{
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uFrameSize);
				}
                pFrame = (MEDIA_VIDEO_FRAME_T *)pData;
				if(pUserData != NULL){
					SysMemory_copy(pUserData, (void *)((PUINT8)pData+sizeof(MEDIA_VIDEO_FRAME_T)), pFrame->stImageFrame.sSize);
				}
				pPool->rIdx = (pPool->rIdx + sizeof(MEDIA_VIDEO_FRAME_T)+pFrame->stImageFrame.sSize) % pPool->totalLen;
				SysMemory_free(pData);
        }
    }
	SysMutex_unlock(pMutex);
    return pFrame->stImageFrame.sSize;
}


unsigned int  GetEncStream(unsigned int uChan,void *pUserData)
{
   
    UINT uLen1 = 0;
	UINT uLen2 = 0;
	UINT uCurLen=0;
    VOID * pData = NULL;
    UINT uR = 0;
	UINT uW = 0;
	UINT uFrameHeaderSize = sizeof(MEDIA_VIDEO_FRAME_T);
	UINT uFrameSize = 0;
	UINT uSize = 0;
    ENC_SHARE_BUF_T *pPool = NULL;
	MEDIA_VIDEO_FRAME_T* pFrame;
    bool bHaveData = true;
	MUTEX_ID* pMutex = NULL;
	if(pManagerParm == NULL)
	{
		return 0;
	}
	if((void *)pManagerParm->aEncPool[uChan].addr[0] == NULL)
	{
		return 0;
	}
	pMutex = &(pManagerParm->aEncPool[uChan].mEncPool);
	SysMutex_lock(pMutex,WAIT_FOREVER);
    while (bHaveData)
    {
        bHaveData = false;
        pPool=&(pManagerParm->aEncPool[uChan]);
		if(pPool == NULL)
		{
		   LOG_ERROR("NO RAWPOOL!\n");
		   return 0;
		}
        uW=pPool->wIdx;
		uR=pPool->rIdx;
        // get data length in share memory
        if (uW >= uR)
        {
                uLen1 = uW - uR;
                uLen2 = 0;
         }
         else
         {
                uLen1 = pPool->totalLen - uR;
                uLen2 = uW;
         }
         uCurLen = uLen1+uLen2;
         if(uCurLen < uFrameHeaderSize)
         {
               LOG_INFO("uCurLen too small uCurLen = %d uFrameHeaderSize = %d\n",uCurLen,uFrameHeaderSize);
         	   SysMutex_unlock(pMutex);
               return 0;
         }
         if(uCurLen >= uFrameHeaderSize)
         {          
                if((void*)pPool->addr[0] == NULL)
                {
                    LOG_ERROR("[chan%d] invalid addr[0] !\n",uChan);
					SysMutex_unlock(pMutex);
                    return 0;
                }
				pData = SysMemory_malloc(uFrameHeaderSize);
				memset((void *)pData,0,uFrameHeaderSize);
				if(uLen1 < uFrameHeaderSize){
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uLen1);
					SysMemory_copy((void*)((PUINT8)pData+uLen1),(void *)(PUINT8)pPool->addr[0], uFrameHeaderSize-uLen1);
				}else{
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uFrameHeaderSize);
				}
				pFrame = (MEDIA_VIDEO_FRAME_T *)pData;	
				uSize = pFrame->stImageFrame.sSize;
				uFrameSize = pFrame->stImageFrame.sSize + uFrameHeaderSize;
				LOG_INFO("frame size:%d all：%d H:%d w:%d\n",uSize,uFrameSize,pFrame->stVideoHeader.u32ImageHeight,pFrame->stVideoHeader.u32ImageWidth);
				if(uCurLen < uFrameSize){
					 SysMemory_free(pData);
					 LOG_INFO("uCurLen too small uCurLen = %d uFrameSize = %d\n",uCurLen,uFrameSize);
         	   		 SysMutex_unlock(pMutex);
               		 return 0;
				}
				SysMemory_free(pData);
				pData = SysMemory_malloc(uFrameSize);
				if(uLen1 < uFrameSize){
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uLen1);
					SysMemory_copy((void*)((PUINT8)pData+uLen1),(void *)(PUINT8)pPool->addr[0], uFrameSize-uLen1);
				}else{
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uFrameSize);
				}
				if(pUserData != NULL){
					SysMemory_copy(pUserData, (void *)((PUINT8)pData+sizeof(MEDIA_VIDEO_FRAME_T)), uSize);
				}
				pPool->rIdx = (pPool->rIdx + uFrameSize) % pPool->totalLen;
				SysMemory_free(pData);
        }
    }
	SysMutex_unlock(pMutex);
    return uSize;
}

unsigned int  GetAudioStream(void *pUserData)
{
   
    UINT uLen1 = 0;
	UINT uLen2 = 0;
	UINT uCurLen=0;
    VOID * pData = NULL;
    UINT uR = 0;
	UINT uW = 0;
	UINT uFrameHeaderSize = sizeof(MEDIA_AUDIO_INFO_T);
	MEDIA_AUDIO_INFO_T* pFrame;
	UINT uFrameSize = 0;
	UINT uSize = 0;
    AUDIO_POOL_INFO_T *pPool = NULL;
    bool bHaveData = true;
	MUTEX_ID* pMutex = NULL;
	if(pManagerParm == NULL)
	{
		return 0;
	}
	if((void *)pManagerParm->stAudioPool.addr[0] == NULL)
	{
		return 0;
	}
	pMutex = &(pManagerParm->stAudioPool.mAudPool);
	SysMutex_lock(pMutex,WAIT_FOREVER);
    while (bHaveData)
    {
        bHaveData = false;
        pPool=&(pManagerParm->stAudioPool);
		if(pPool == NULL)
		{
		   LOG_ERROR("NO RAWPOOL!\n");
		   return 0;
		}
        uW=pPool->wIdx;
		uR=pPool->rIdx;
        // get data length in share memory
        if (uW >= uR)
        {
                uLen1 = uW - uR;
                uLen2 = 0;
         }
         else
         {
                uLen1 = pPool->totalLen - uR;
                uLen2 = uW;
         }
         uCurLen = uLen1+uLen2;
         if(uCurLen < uFrameHeaderSize)
         {
               LOG_INFO("uCurLen too small uCurLen = %d uFrameHeaderSize = %d\n",uCurLen,uFrameHeaderSize);
         	   SysMutex_unlock(pMutex);
               return 0;
         }
         if(uCurLen >= uFrameHeaderSize)
         {          
                if((void*)pPool->addr[0] == NULL)
                {
                    LOG_ERROR("invalid addr[0] !\n");
					SysMutex_unlock(pMutex);
                    return 0;
                }
				pData = SysMemory_malloc(uFrameHeaderSize);
				memset((void *)pData,0,uFrameHeaderSize);
				if(uLen1 < uFrameHeaderSize){
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uLen1);
					SysMemory_copy((void*)((PUINT8)pData+uLen1),(void *)(PUINT8)pPool->addr[0], uFrameHeaderSize-uLen1);
				}else{
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uFrameHeaderSize);
				}
				pFrame = (MEDIA_AUDIO_INFO_T *)pData;	
				uSize = pFrame->iFrameLen;
				uFrameSize = pFrame->iFrameLen + uFrameHeaderSize;
				if(uCurLen < uFrameSize){
					 SysMemory_free(pData);
					 LOG_INFO("uCurLen too small uCurLen = %d uFrameSize = %d\n",uCurLen,uFrameSize);
         	   		 SysMutex_unlock(pMutex);
               		 return 0;
				}
				SysMemory_free(pData);
				pData = SysMemory_malloc(uFrameSize);
				if(uLen1 < uFrameSize){
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uLen1);
					SysMemory_copy((void*)((PUINT8)pData+uLen1),(void *)(PUINT8)pPool->addr[0], uFrameSize-uLen1);
				}else{
					SysMemory_copy(pData,(void *)((PUINT8)pPool->addr[0] + uR), uFrameSize);
				}
				if(pUserData != NULL){
					SysMemory_copy(pUserData, (void *)((PUINT8)pData+sizeof(MEDIA_VIDEO_FRAME_T)), uSize);
				}
				pPool->rIdx = (pPool->rIdx + uFrameSize) % pPool->totalLen;
				SysMemory_free(pData);
        }
    }
	SysMutex_unlock(pMutex);
    return uSize;
}



