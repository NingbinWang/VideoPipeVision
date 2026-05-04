#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "AudioMediaSource.h"
#include "New.h"
#include "Logger.h"
#include "MediaManager.h"
#include "SysMemory.h"
#include "SysTime.h"
#include "MediaSource.h"

#ifndef VIDEOBUFFERSIZE
#define VIDEOBUFFERSIZE (1024*1024)
#endif

#ifndef DEFAULT_FRAME_NUM
#define DEFAULT_FRAME_NUM 10
#endif
AudioMediaSource* AudioMediaSource::createNew(UsageEnvironment* env)
{
    return New<AudioMediaSource>::allocate(env);
}

AudioMediaSource::AudioMediaSource(UsageEnvironment* env) :
    MediaSource(env),
    mEnv(env)
    
{
    this->mOutputbuf = (char *)SysMemory_malloc(VIDEOBUFFERSIZE);
    if(this->mOutputbuf == NULL){
		LOG_ERROR("NO MEM\n");
    }
    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mEnv->threadPool()->addTask(mTask);
        mEnv->threadPool()->addTask(mTask);
    LOG_DEBUG("AudioMediaSource OK\n");
}

AudioMediaSource::~AudioMediaSource()
{
    if(this->mOutputbuf != nullptr)
        SysMemory_free(this->mOutputbuf);
}

void AudioMediaSource::readFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);
    size_t size = 0;
    if(mAvFrameInputQueue.empty())
        return;
    AvFrame* frame = mAvFrameInputQueue.front();
    while(1)
	{
		size = GetAudioStream(this->mOutputbuf);
		if(size == 0)
		{
            LOG_INFO("don't have one framebuf\n");
            SysTime_sleep_ms(33);
            continue;
		}
		break;
    }
    memcpy(frame->mBuffer, this->mOutputbuf, size);
    frame->mFrame = frame->mBuffer;
    frame->mFrameSize = size;
    mAvFrameInputQueue.pop();
    mAvFrameOutputQueue.push(frame);
}




