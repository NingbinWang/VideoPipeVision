#include "MediaSource.h"
#include "Logger.h"
#include "New.h"

MediaSource::MediaSource(UsageEnvironment* env) :
    mEnv(env)
{
    mMutex = Mutex::createNew();
    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mAvFrameInputQueue.push(&mAvFrames[i]);
    
    mTask.setTaskCallback(taskCallback, this);
}

MediaSource::~MediaSource()
{
    //delete mMutex;
    Delete::release(mMutex);
}

AvFrame* MediaSource::getFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);

    if(mAvFrameOutputQueue.empty())
    {
        return NULL;
    }

    AvFrame* frame = mAvFrameOutputQueue.front();    
    mAvFrameOutputQueue.pop();

    return frame;
}

void MediaSource::putFrame(AvFrame* frame)
{
    MutexLockGuard mutexLockGuard(mMutex);
    mAvFrameInputQueue.push(frame);
    mEnv->threadPool()->addTask(mTask);
}


void MediaSource::taskCallback(void* arg)
{
    MediaSource* source = (MediaSource*)arg;
    source->readFrame();
}
