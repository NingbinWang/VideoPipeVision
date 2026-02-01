#include "MediaViInner.h"
#include "MediaPriv.h"
MediaVi* MediaVi::createNew(VI_CFG_PARAM_T&pParams)
{
    //return new RtspServer(env, addr);
    return New<MediaVi>::allocate(&pParams);
}
MediaVi::MediaVi(VI_CFG_PARAM_T *pParams) :
mpParams(pParams)
{
	if(mpParams->u32Enable == FALSE){
		return;
	}
	if(mpParams->eViType == VI_V4L2){
		INT32 i32Format =  V4l2Device::fourcc(mpParams->strFormat);
		V4L2DeviceParameters param(mpParams->strDevname,i32Format, mpParams->u32Image_viW, mpParams->u32Image_viH,mpParams->u32Frame_rate, IOTYPE_MMAP);
        mpCapture = V4l2Capture::create(param,V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
        if (mpCapture == NULL)
        {
             LOG_WARNING("Cannot reading from V4L2 capture interface for device:%s\n",mpParams->strDevname);
             return;
        }
        LOG_DEBUG("V4L2 capture OK! device:%s\n",mpParams->strDevname);
	}
	mpMutex = Mutex::createNew();
}

MediaVi::~MediaVi() 
{
	
}

INT32 MediaVi::GetFrameFromQueue(MEDIA_VIDEO_FRAME_T *pstFrameInfo, INT32 timeout)
{
	if(pstFrameInfo == NULL){
		return ERROR;
	}
	if(mpParams->u32Enable == FALSE){
		LOG_WARNING("chn %d is not enable\n",mpParams->u32Chn);
		return OK;
	}
	MutexLockGuard mutexLockGuard(mpMutex);
	pstFrameInfo->stVideoHeader.u32ImageHeight = mpParams->u32Image_viH;
	pstFrameInfo->stVideoHeader.u32ImageWidth = mpParams->u32Image_viW;
	pstFrameInfo->stVideoHeader.eFormatType = MediaForccFrame(mpParams->strFormat);
	if(mpCapture->capturepoll(timeout)){
#ifdef MEDIARKMPP
		pstFrameInfo->stImageFrame.pPhyAddr = mpCapture->readFrame(&pstFrameInfo->stVideoHeader.uIndex);
		pstFrameInfo->stImageFrame.pVirAddr = mpp_buffer_get_ptr(pstFrameInfo->stImageFrame.pPhyAddr);
		pstFrameInfo->stVideoHeader.iframeNum = mpp_buffer_get_index((MppBuffer)pstFrameInfo->stImageFrame.pPhyAddr);
		pstFrameInfo->stImageFrame.sSize = mpp_buffer_get_size((MppBuffer)pstFrameInfo->stImageFrame.pPhyAddr);
		mpp_buffer_sync_end(pstFrameInfo->stImageFrame.pPhyAddr);
#endif
	}
	
	return OK;
}

BOOL MediaVi::PutFrameInQueue(MEDIA_VIDEO_FRAME_T *pstFrameInfo)
{
	return mpCapture->releaseFrame(pstFrameInfo->stVideoHeader.uIndex);
}

int MediaVi::readFramebuf(char* buffer, int bufferSize)
{
    int ret = -1 ;
    ret = mpCapture->read(buffer,bufferSize);
    return ret;
}

bool MediaVi::poll()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return mpCapture->isReadable(&tv);
}
