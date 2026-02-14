#pragma once 
#include <string>
#include <queue>
#include <stdint.h>
#include "Media.h"
#include "Logger.h"
#include "MediaSource.h"

class AudioMediaSource : public MediaSource
{
public:
		static AudioMediaSource* createNew(UsageEnvironment* env);
		AudioMediaSource(UsageEnvironment* env);
		virtual ~AudioMediaSource();
protected:
    virtual void readFrame();

private:
	UsageEnvironment* mEnv;
    char *mOutputbuf=nullptr;
    char *mFramebuf=nullptr;
};