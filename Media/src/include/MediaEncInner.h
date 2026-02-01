#ifndef _MEDIAENCINNER_H_
#define _MEDIAENCINNER_H_
#include "Media.h"
#include "MediaFrame.h"
#include "Logger.h"
#include "Common.h"
#include <thread>
#include "MppEncoder.h"
#include "SysMutex.h"

#define MPPENCOERSIZE             (5*1024*1024)

typedef void (*MediaEncCallback)(void* userdata, const char* data, int size);

class MediaEnc
{
public:
	static MediaEnc* createNew(MEDIA_ENC_PARAM_T&Params);
    MediaEnc(MEDIA_ENC_PARAM_T* pParams);
	~MediaEnc();
	int MediaEncEncode(void* mpp_buf, char* enc_buf, int max_size);
	int MediaEncGetHeader(char* enc_buf, int max_size);
	int MediaEncSetCallback(MediaEncCallback callback );
	size_t MediaEncGetFrameSize();
	void* MediaEncGetInputFrame();
	int MediaEncGetInputFrameBufferFd(void * source);
	void* MediaEncGetInputFrameBufferAddr(void * source);
	MppFrameFormat MediaEncFmtTranslation(MEDIA_FORMAT_TYPE_E eType);
	MppCodingType MediaEncTypeTranslation(MEDIA_ENC_CODETYPE_E eType);
	MppEncRcMode MediaEncTypeTranslation(MEDIA_ENC_RCMODE_E eType);
	VOID MediaEncStopThread();
	VOID MediaEncGetStreamFunc(VOID* pUserdata);
	VOID MediaEncStartThread();
private:
	VOID  MediaEncSendStream(MEDIA_ENC_FRAME_T* pFrame);
	INT32 MediaEncStartCode(const CHAR *buf,UINT32 pos);
	INT32 MediaEncParseH264(MEDIA_ENC_FRAME_T *pEncFrame);
	UINT32       muChan;
	MppEncoder * mpEncoder;
	std::thread  mThread;
	bool bRunning{false};
	MEDIA_ENC_PARAM_T* mpParams;
	MUTEX_ID mtxEnc;
};


#endif

