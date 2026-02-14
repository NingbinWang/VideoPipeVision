#ifndef _MEDIAVIINNER_H_
#define _MEDIAVIINNER_H_
#include "Media.h"
#include "MediaFrame.h"
#include "Logger.h"
#include "Common.h"
#include "V4l2Device.h"
#include "V4l2Capture.h"
#include "New.h"
#include "Mutex.h"


class MediaVi
{
public:
    static MediaVi* createNew(VI_CFG_PARAM_T&Params);
    MediaVi(VI_CFG_PARAM_T* pParams);
    ~MediaVi();
    int readFramebuf(char* buffer, int bufferSize);
    bool poll();
	INT32 GetFrameFromQueue(MEDIA_VIDEO_FRAME_T *pstFrameInfo, INT32 timeout);
	BOOL PutFrameInQueue(MEDIA_VIDEO_FRAME_T *pstFrameInfo);

private:
    VI_CFG_PARAM_T* mpParams;
    Mutex* mpMutex;
    V4l2Capture* mpCapture;
	// 内部辅助函数：计算Stride
    INT32 calculateWidthStride(UINT32 width, MEDIA_FORMAT_TYPE_E format, UINT32 alignment = 32);
    INT32 calculateHeightStride(UINT32 height, MEDIA_FORMAT_TYPE_E format, UINT32 alignment = 1);
};

#endif

