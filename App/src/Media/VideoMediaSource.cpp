#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "VideoMediaSource.h"
#include "New.h"
#include "Logger.h"
#include "MediaManager.h"
#include "SysMemory.h"
#include "SysTime.h"
VideoMediaSource* VideoMediaSource::createNew(UsageEnvironment* env)
{
    return New<VideoMediaSource>::allocate(env);
}

VideoMediaSource::VideoMediaSource(UsageEnvironment* env) :
    MediaSource(env),
    mEnv(env)
    
{
   this->mOutputbuf = (char *)SysMemory_malloc(VIDEOBUFFERSIZE);
   if(this->mOutputbuf == NULL){
		LOG_ERROR("NO MEM\n");
   }
   setFps(30);
   for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
       mEnv->threadPool()->addTask(mTask);
    LOG_DEBUG("VideoMediaSource OK\n");
}

VideoMediaSource::~VideoMediaSource()
{
    if(this->mOutputbuf != nullptr)
        SysMemory_free(this->mOutputbuf);

}

static inline int startCode3(uint8_t* buf)
{
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

void VideoMediaSource::readFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);
    char * framebuf = nullptr;
    size_t size = 0;
    if(mAvFrameInputQueue.empty())
        return;
    AvFrame* frame = mAvFrameInputQueue.front();
   
    if(mNaluQueue.empty())
    {
    	while(1)
		{
			size = GetEncStream(0,this->mOutputbuf);
			if(size == 0)
			{
			 LOG_INFO("don't have one framebuf\n");
			 SysTime_sleep_ms(33);
             continue;
			}
			break;
    	}
		framebuf = (char *)SysMemory_malloc(size);
        memcpy(framebuf, this->mOutputbuf, size);
        memset(this->mOutputbuf,0,size);
        mNaluQueue.push(Nalu((uint8_t*)framebuf, size));
    }
    Nalu nalu = mNaluQueue.front();
    mNaluQueue.pop();
    memcpy(frame->mBuffer, nalu.mData, nalu.mSize);
    if(startCode3(nalu.mData))
    {
        frame->mFrame = frame->mBuffer+3;
        frame->mFrameSize = nalu.mSize-3;
    }
    else
    {
        frame->mFrame = frame->mBuffer+4;
        frame->mFrameSize = nalu.mSize-4;
    }
    SysMemory_free(nalu.mData);
    mAvFrameInputQueue.pop();
    mAvFrameOutputQueue.push(frame);
}

