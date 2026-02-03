#include "MediaEncInner.h"
#include "SysMemory.h"
#include "MediaPriv.h"
#include "SysTime.h"

int  MediaEnc::MediaEncEncode(void* mpp_buf, char* enc_buf, int max_size)
{
   return mpEncoder->Encode(mpp_buf,enc_buf,max_size);
}


int  MediaEnc::MediaEncGetHeader(char* enc_buf, int max_size)
{
   return mpEncoder->GetHeader(enc_buf,max_size);
}

int  MediaEnc::MediaEncSetCallback(MediaEncCallback callback )
{
   return mpEncoder->SetCallback((MppEncoderFrameCallback)callback);
}

size_t MediaEnc::MediaEncGetFrameSize()
{
   return mpEncoder->GetFrameSize();
}

void* MediaEnc::MediaEncGetInputFrame()
{
   return mpEncoder->GetInputFrameBuffer();
}

int MediaEnc::MediaEncGetInputFrameBufferFd(void * source)
{
   return mpEncoder->GetInputFrameBufferFd(source);
}

void* MediaEnc::MediaEncGetInputFrameBufferAddr(void * source)
{
   return mpEncoder->GetInputFrameBufferAddr(source);
}


MppFrameFormat MediaEnc::MediaEncFmtTranslation(MEDIA_FORMAT_TYPE_E eType)
{
    switch(eType){
			case MEDIA_FORMAT_YUV420SP:
				return MPP_FMT_YUV420SP;
			default:
				return MPP_FMT_BUTT;
    }
	return MPP_FMT_BUTT;

}

MppCodingType MediaEnc::MediaEncTypeTranslation(MEDIA_ENC_CODETYPE_E eType)
{
	
	switch(eType){
		case MEDIA_ENC_CODETYPE_AVC:
			return MPP_VIDEO_CodingAVC;
		default:
			return MPP_VIDEO_CodingMax;
	}
	return MPP_VIDEO_CodingMax;
}

MppEncRcMode MediaEnc::MediaEncTypeTranslation(MEDIA_ENC_RCMODE_E eType)
{
	switch(eType){
			case MEDIA_ENC_RCMODE_VBR:
				return MPP_ENC_RC_MODE_VBR;
			default:
				return MPP_ENC_RC_MODE_BUTT;
		}
	return MPP_ENC_RC_MODE_BUTT;
}

MediaEnc* MediaEnc::createNew(MEDIA_ENC_PARAM_T&pParams)
{
    return New<MediaEnc>::allocate(&pParams);
}
MediaEnc::MediaEnc(MEDIA_ENC_PARAM_T *pParams):
mpParams(pParams)
{
	 MEDIA_FORMAT_TYPE_E eType ;
	 MppEncoderParams stEncoderParam;
	 memset(&stEncoderParam,0,sizeof(MppEncoderParams));
	 eType = MediaForccFrame(pParams->strStreamType);
	 stEncoderParam.width = (RK_U32)pParams->uEncW;
     stEncoderParam.height = (RK_U32)pParams->uEncH;
     stEncoderParam.fmt = MediaEncFmtTranslation(eType);
     stEncoderParam.type = MediaEncTypeTranslation(pParams->eEncoderType);
     stEncoderParam.rc_mode =  MediaEncTypeTranslation(pParams->eRcMode);
	 mpEncoder = Media_Getmppencoder();
	 mpEncoder->Init(stEncoderParam,nullptr);//Encoder目前只有一个所以这是要修正的
	 muChan = pParams->uChan;
	 SysMutex_create(&this->mtxEnc, 0);
	 LOG_INFO("chan%d enc ok!\n",muChan);
}

MediaEnc::~MediaEnc() 
{
	
}


INT32 MediaEnc::MediaEncStartCode(const CHAR *buf,UINT32 pos) {
    // 检查 4字节起始码 0x00 00 00 01
    if (buf[pos+0] == 0x00 && buf[pos+1] == 0x00 && 
        buf[pos+2] == 0x00 && buf[pos+3] == 0x01) {
        return 4;
    }
    // 检查 3字节起始码 0x00 00 01
    else if (buf[pos+0] == 0x00 && buf[pos+1] == 0x00 && 
             buf[pos+2] == 0x01) {
        return 3;
    }
    return ERROR; // 未找到
}


INT32 MediaEnc::MediaEncParseH264(MEDIA_ENC_FRAME_T *pEncFrame)
{
	INT32 iRet = ERROR;
	UINT32 u32pos = 0;
	//UINT32 u32Startpos = u32pos;
	UINT32 u32Endpos = u32pos;
	UINT32 u32NaluLen = 0;
	//NALHEADER_T  stNaluHeader;
	UINT32  u32NaluType = 0;
	UINT32 u32len = (UINT32)pEncFrame->stVideoFrame.stImageFrame.sSize;
	CHAR* pH264Buf = (CHAR*)pEncFrame->stVideoFrame.stImageFrame.pVirAddr;
	// 遍历缓冲区
    while (u32pos < u32len - 4) {
        // 查找起始码 0x00000001
        if (MediaEncStartCode(pH264Buf,u32pos) == 4) {
			pEncFrame->u32NaluNum++;
            u32pos += 4; // 跳过起始码，指向 NALU Header
            // 找到下一个起始码，确定当前 NALU 的长度
            u32Endpos = u32pos;
            while (u32Endpos < u32len - 4) {
                if (MediaEncStartCode(pH264Buf+u32Endpos,u32pos) == 4) {
                    break;
                }
                u32Endpos++;
            }
            // 计算 NALU 长度 (不含起始码，只含 Header + Payload)
            u32NaluLen = u32Endpos - u32pos;
            // 提取 NALU Header (第一个字节)
            //stNaluHeader = (NALHEADER_T *)pH264Buf[u32pos];
            // 解析 NALU 类型
            //u32NaluType = (UINT32)stNaluHeader.uNalUnitType;
			if(pEncFrame->u32NaluNum == MAX_NALU_NUM){
				break;
			}
			pEncFrame->astNalu[pEncFrame->u32NaluNum].u32NaluLen = u32NaluLen;
			pEncFrame->astNalu[pEncFrame->u32NaluNum].u32NaluType = u32NaluType;
			//pEncFrame->astNalu[pEncFrame->u32NaluNum].pNaluPtr = (VOID*)pH264Buf+u32pos;
            // 移动 pos 指针到下一个 NALU 的起始位置
            u32pos = u32Endpos;
        } else {
            u32pos++;
        }
    }
    
    // 处理粘包/断包情况：将未处理完的数据移到缓冲区头部
    if (u32pos < u32len) {
        memmove(pH264Buf, pH264Buf + u32pos, u32len - u32pos);
        return u32len - u32pos;
    }
    return iRet;
}

VOID MediaEnc::MediaEncSendStream(MEDIA_ENC_FRAME_T* pFrame)
{	
	MEDIA_INNER_PARAM_T* pInnerParam =  Media_Get_InnerParam();
	CHAR *pOutputbuf = (CHAR*)SysMemory_malloc(MPPENCOERSIZE);
	MEDIA_VIDEO_FRAME_T stEncFrameInfo;
	UINT32 uFrameSize = 0;
	memset((void *)&stEncFrameInfo, 0, sizeof(MEDIA_VIDEO_FRAME_T));
	stEncFrameInfo.stImageFrame.sSize = pFrame->stVideoFrame.stImageFrame.sSize;
	stEncFrameInfo.stVideoHeader.eFormatType =  pFrame->stVideoFrame.stVideoHeader.eFormatType;
	stEncFrameInfo.stVideoHeader.iframeNum =  pFrame->stVideoFrame.stVideoHeader.iframeNum;
	stEncFrameInfo.stVideoHeader.u32ImageHeight =  pFrame->stVideoFrame.stVideoHeader.u32ImageHeight;
	stEncFrameInfo.stVideoHeader.u32ImageWidth =  pFrame->stVideoFrame.stVideoHeader.u32ImageWidth;
	stEncFrameInfo.stImageFrame.pVirAddr = pFrame->stVideoFrame.stImageFrame.pVirAddr; 
	SysMemory_copy((VOID *)pOutputbuf, &stEncFrameInfo,sizeof(MEDIA_VIDEO_FRAME_T));
	SysMemory_copy((VOID *)(pOutputbuf+sizeof(MEDIA_VIDEO_FRAME_T)),pFrame->stVideoFrame.stImageFrame.pVirAddr,stEncFrameInfo.stImageFrame.sSize);
	uFrameSize = stEncFrameInfo.stImageFrame.sSize+sizeof(MEDIA_VIDEO_FRAME_T);
	pInnerParam->pStream->SendStreamToEncPool(0, (PUINT8)pOutputbuf, uFrameSize,0);
	SysMemory_free(pOutputbuf);
}




VOID MediaEnc::MediaEncGetStreamFunc(void* pUserdata)
{
	//INT32 iRet = ERROR;
	MEDIA_INNER_PARAM_T* pInnerParam =  Media_Get_InnerParam();
	MEDIA_VIDEO_FRAME_T stFrameInfo;
	MEDIA_ENC_FRAME_T   stEncFrame;
	MEDIA_ENC_STATUS_T*  pEncStatus = &mpParams->stEncStatus;
	memset((void *)&stFrameInfo, 0, sizeof(MEDIA_VIDEO_FRAME_T));
	memset((void *)&stEncFrame, 0, sizeof(MEDIA_ENC_FRAME_T));
	pEncStatus->bStart = TRUE;
	pEncStatus->uEncType = mpParams->eEncoderType;
	stEncFrame.stVideoFrame.stImageFrame.pVirAddr = SysMemory_malloc(MPPENCOERSIZE);
	stEncFrame.uChan = muChan;
	//stEncFrame.stVideoFrame.stVideoHeader.eFormatType =  mpParams->eEncoderType;
	stEncFrame.stVideoFrame.stVideoHeader.u32ImageHeight =  mpParams->uEncH;
	stEncFrame.stVideoFrame.stVideoHeader.u32ImageWidth =  mpParams->uEncW;
	pEncStatus->uEncW = mpParams->uEncW;
	pEncStatus->uEncH = mpParams->uEncH;
	while(bRunning){
		SysMutex_lock(&this->mtxEnc,1000);
		pInnerParam->apVi[muChan]->GetFrameFromQueue(&stFrameInfo, 1000);
		stFrameInfo.stVideoHeader.uChan = muChan;
		pEncStatus->uEncFrm = stFrameInfo.stVideoHeader.iframeNum;
	    //配置OSD时间
	    SysTime_get_msec(&stEncFrame.u64TimeStamp);//获取时间戳
	    //进行OSD叠加
	   	//设置AI回调
	    //调用给原始画面换成VO能够被识别出来的数据
	    pInnerParam->pStream->StreamFrameConvert(&stFrameInfo,0);
	    //送去编码
	    //audio编码
	     //pInnerParam->pAudio->MediaAudioSendStream(&stAudioInfo);
		//视频编码
		stEncFrame.stVideoFrame.stImageFrame.sSize =  MediaEnc::MediaEncEncode(stFrameInfo.stImageFrame.pPhyAddr,(char *)stEncFrame.stVideoFrame.stImageFrame.pVirAddr,MPPENCOERSIZE);
		stEncFrame.stVideoFrame.stVideoHeader.iframeNum = pEncStatus->uEncFrm;
		//MediaEncParseH264(&stEncFrame);
		//送到编码共享流
		MediaEncSendStream(&stEncFrame);
        //PS封装送流
		pInnerParam->pStream->StreamPack(&stEncFrame);
		
		pInnerParam->apVi[muChan]->PutFrameInQueue(&stFrameInfo);
		SysMutex_unlock(&this->mtxEnc);
	}
	SysMemory_free(stEncFrame.stVideoFrame.stImageFrame.pVirAddr);
}



VOID MediaEnc::MediaEncStopThread() {
    	bRunning = false;
        if (mThread.joinable()) {
            mThread.join();
        }
    }


VOID MediaEnc::MediaEncStartThread() 
{
	bRunning = true;
	mThread = std::thread([this]() {
		MediaEncGetStreamFunc(NULL);
	});

}


