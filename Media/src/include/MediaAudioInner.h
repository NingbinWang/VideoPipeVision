#ifndef _MEDIAAUDIOINNER_H_
#define _MEDIAAUDIOINNER_H_
#include "Media.h"
#include "Logger.h"
#include "Common.h"
#include "MediaAudio.h"
#include "MediaPriv.h"
#include <alsa/asoundlib.h>
#include <fdk-aac/aacenc_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class MediaAudio
{
public:
	static MediaAudio* createNew(AUDIO_CFG_PARAM_T&Params);
    MediaAudio(AUDIO_CFG_PARAM_T* pParams);
	~MediaAudio();
	INT32 MediaAudioRecordInit(const char* strDevName);
	INT32 MediaAudioAacEncoder(PUINT8 pAsc,UINT uSize);
	INT32 MediaAudioSendAccStraem(VOID);
	
private:
	snd_pcm_t *mpPcmhandle;
    snd_pcm_hw_params_t *mpPcmParams;
	snd_pcm_uframes_t mFrameSize;
	HANDLE_AACENCODER mAacEncoder;
	UINT32       muChan;
	AUDIO_CFG_PARAM_T* mpParams;
	INT16 *pPcmBuffer;
	MUTEX_ID mtxAudio;
};



#endif
