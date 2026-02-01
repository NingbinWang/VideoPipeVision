#ifndef _MEDIA_SOURCE_H_
#define _MEDIA_SOURCE_H_
#include <queue>
#include <stdint.h>

#include "UsageEnvironment.h"
#include "Mutex.h"

#define FRAME_MAX_SIZE (1024*1024*50)
#define DEFAULT_FRAME_NUM 5

class AvFrame
{
public:
    AvFrame() :
        mBuffer(new uint8_t[FRAME_MAX_SIZE]),
        mFrameSize(0)
    { }

    ~AvFrame()
    { delete mBuffer; }

    uint8_t* mBuffer;
    uint8_t* mFrame;
    int mFrameSize;
};

class MediaSource
{
public:
    virtual ~MediaSource();

    AvFrame* getFrame();
    void putFrame(AvFrame* frame);
    int getFps() const { return mFps; }

protected:
    MediaSource(UsageEnvironment* env);
    virtual void readFrame() = 0;
    void setFps(int fps) { mFps = fps; }

private:
    static void taskCallback(void*);

protected:
    UsageEnvironment* mEnv;
    AvFrame mAvFrames[DEFAULT_FRAME_NUM];
    std::queue<AvFrame*> mAvFrameInputQueue;
    std::queue<AvFrame*> mAvFrameOutputQueue;
    Mutex* mMutex;
    ThreadPool::Task mTask;
    int mFps;
};

#endif //_MEDIA_SOURCE_H_