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

// 计算宽度Stride的核心函数
INT32 MediaVi::calculateWidthStride(UINT32 width, MEDIA_FORMAT_TYPE_E format, UINT32 alignment) {
    UINT32 bytesPerPixel;
    
    // 根据格式确定每像素字节数
    switch (format) {
        case MEDIA_FORMAT_YUV420SP:  // NV12
        case MEDIA_FORMAT_YUV420SP_10BIT:
        case MEDIA_FORMAT_YUV422SP:  // NV16
        case MEDIA_FORMAT_YUV422SP_10BIT:
        case MEDIA_FORMAT_YUV420SP_VU:  // NV21
        case MEDIA_FORMAT_YUV422SP_VU:  // NV61
            // 对于NV格式，Y平面和UV平面的宽度Stride相同
            bytesPerPixel = 1; // Y平面
            break;
            
        case MEDIA_FORMAT_YUV420P:  // I420
        case MEDIA_FORMAT_YUV422P:
        case MEDIA_FORMAT_YUV444P:
        case MEDIA_FORMAT_YUV440SP:
        case MEDIA_FORMAT_YUV411SP:
        case MEDIA_FORMAT_YUV444SP:
        case MEDIA_FORMAT_YUV444SP_10BIT:
            // 对于平面格式，Y平面的bytesPerPixel=1
            bytesPerPixel = 1;
            break;
            
        case MEDIA_FORMAT_YUV422_YUYV:  // YUY2
        case MEDIA_FORMAT_YUV422_YVYU:  // YVY2
        case MEDIA_FORMAT_YUV422_UYVY:  // UYVY
        case MEDIA_FORMAT_YUV422_VYUY:  // VYUY
            bytesPerPixel = 2; // 每2个像素包含YUYV数据
            break;
            
        case MEDIA_FORMAT_RGB888:  // RGB24
        case MEDIA_FORMAT_BGR888:  // BGR24
            bytesPerPixel = 3;
            break;
            
        case MEDIA_FORMAT_RGB565:  // RGB565
        case MEDIA_FORMAT_BGR565:  // BGR565
        case MEDIA_FORMAT_RGB555:  // RGB555
        case MEDIA_FORMAT_BGR555:  // BGR555
        case MEDIA_FORMAT_RGB444:  // RGB444
        case MEDIA_FORMAT_BGR444:  // BGR444
            bytesPerPixel = 2;
            break;
            
        case MEDIA_FORMAT_ARGB8888:  // ARGB32
        case MEDIA_FORMAT_ABGR8888:  // ABGR32
        case MEDIA_FORMAT_BGRA8888:  // BGRA32
        case MEDIA_FORMAT_RGBA8888:  // RGBA32
            bytesPerPixel = 4;
            break;
            
        case MEDIA_FORMAT_YUV400:  // 仅Y
            bytesPerPixel = 1;
            break;
            
        case MEDIA_FORMAT_RAW8:   // RAW8
            bytesPerPixel = 1;
            break;
            
        case MEDIA_FORMAT_RAW10:  // RAW10
        case MEDIA_FORMAT_RAW12:  // RAW12
        case MEDIA_FORMAT_RAW14:  // RAW14
            // RAW格式通常以字节为单位计算，但实际像素数需要考虑位深度
            bytesPerPixel = 2; // 通常RAW10/12/14会打包成字节
            break;
            
        case MEDIA_FORMAT_RAW16:  // RAW16
            bytesPerPixel = 2;
            break;
            
        case MEDIA_FORMAT_YUV_BUTT:
        case MEDIA_FORMAT_RGB_BUTT:
        case MEDIA_FORMAT_Y_ONLY:
        case MEDIA_FORMAT_BGR_PACK_Y:
        case MEDIA_FORMAT_MAX:
        default:
            // 默认使用RGB24的字节数
            bytesPerPixel = 3;
            break;
    }
    
    // 计算原始字节数
    UINT32 rawBytes = width * bytesPerPixel;
    
    // 对齐到指定字节边界
    UINT32 alignedBytes = ((rawBytes + alignment - 1) / alignment) * alignment;
    
    return (INT32)alignedBytes;
}

// 计算高度Stride的核心函数
INT32 MediaVi::calculateHeightStride(UINT32 height, MEDIA_FORMAT_TYPE_E format, UINT32 alignment) {
    // 高度Stride通常就是图像的实际高度，但在某些情况下可能需要对齐
    UINT32 alignedHeight = ((height + alignment - 1) / alignment) * alignment;
    
    return (INT32)alignedHeight;
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
	    // 计算并设置Stride信息
    pstFrameInfo->stVideoHeader.iWidthStride = calculateWidthStride(
        mpParams->u32Image_viW, 
        pstFrameInfo->stVideoHeader.eFormatType
    );
    pstFrameInfo->stVideoHeader.iHeightStride = calculateHeightStride(
        mpParams->u32Image_viH, 
        pstFrameInfo->stVideoHeader.eFormatType
    );
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
