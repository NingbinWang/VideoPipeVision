#ifndef _MEDIAAUDIOINNER_H_
#define _MEDIAAUDIOINNER_H_
#include "Media.h"
#include "Logger.h"
#include "Common.h"
#include "MediaAudio.h"
#include <alsa/asoundlib.h>
#include <fdk-aac/aacenc_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "New.h"
#define AUDIOBUFFERSIZE  8192
class MediaAudio
{
public:
	static MediaAudio* createNew(AUDIO_CFG_PARAM_T&Params);
    MediaAudio(AUDIO_CFG_PARAM_T* pParams);
	~MediaAudio();
	INT32 MediaAudioRecordInit(const char* strDevName);
	INT32 MediaAudioAacEncoder(AACENC_INFO_T* pInfo);
	INT32 MediaAudioGetAacData(PUINT8 pAacdata);
	INT32 MediaAudioEncPcmToAac(INT16 *pPcmSamples, UINT uNumSamples, UINT8 *pOutData);
	INT32 MediaAudioSendStream(MEDIA_AUDIO_INFO_T* pstAudioInfo);
	
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
