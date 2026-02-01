
#pragma once 
#include <string>
#include <queue>
#include <stdint.h>
#include "Media.h"
#include "Logger.h"
#include "MediaSource.h"
#include "x264.h"


#define VIDEOBUFFERSIZE 5*1024*1024
class VideoMediaSource : public MediaSource
{
public:
    static VideoMediaSource* createNew(UsageEnvironment* env);
    
    VideoMediaSource(UsageEnvironment* env);
    virtual ~VideoMediaSource();

protected:
    virtual void readFrame();

private:
    struct Nalu
    {
        Nalu(uint8_t* data, int size) : mData(data), mSize(size)
        { }

        uint8_t* mData;
        int mSize;
    };



private:
    UsageEnvironment* mEnv;
    char *mOutputbuf=nullptr;
    std::queue<Nalu> mNaluQueue;
};