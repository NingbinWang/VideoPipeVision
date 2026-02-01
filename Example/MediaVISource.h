
#pragma once 
#include <string>
#include <queue>
#include <stdint.h>
#include "Media.h"
#include "Logger.h"
#include "MediaSource.h"
#include "x264.h"



class MediaVISource : public MediaSource
{
public:
    static MediaVISource* createNew(UsageEnvironment* env, std::string dev);
    
    MediaVISource(UsageEnvironment* env, const std::string& dev);
    virtual ~MediaVISource();

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
    bool x264Init();
    bool x264Exit();



private:
    UsageEnvironment* mEnv;
    std::string mDev;
    int mWidth;
    int mHeight;
    char *mFramebuf=nullptr;
    
    x264_nal_t* mNals;
	x264_t* mX264Handle;
	x264_picture_t* mPicIn;
	x264_picture_t* mPicOut;
	x264_param_t* mParam;
    int mCsp;
    int mPts;
    std::queue<Nalu> mNaluQueue;
};