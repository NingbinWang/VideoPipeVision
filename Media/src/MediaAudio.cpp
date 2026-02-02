#include "MediaAudioInner.h"
#include "SysMemory.h"

MediaAudio* MediaAudio::createNew(AUDIO_CFG_PARAM_T&pParams)
{
    return New<MediaAudio>::allocate(&pParams);
}
MediaAudio::MediaAudio(AUDIO_CFG_PARAM_T *pParams):
mpParams(pParams)
{

	 muChan = pParams->uChan;
	 mFrameSize = pParams->uPcmFrameSize;
	 MediaAudioRecordInit(pParams->strDevname);
	 SysMutex_create(&this->mtxAudio, 0);
	 
}

MediaAudio::~MediaAudio() 
{
	if(pPcmBuffer){
	  SysMemory_free(pPcmBuffer);
	}
	if(this->mpPcmhandle){
		snd_pcm_close(this->mpPcmhandle);
	}
	aacEncClose(&mAacEncoder);
}

INT32 MediaAudio::MediaAudioRecordInit(const char* strDevName)
{	
	 // 打开 ALSA 设备
    if (snd_pcm_open(&mpPcmhandle, strDevName, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        LOG_ERROR("Cannot open audio device %s\n", strDevName);
        return ERROR;
    }
    snd_pcm_hw_params_alloca(&this->mpPcmParams);
    snd_pcm_hw_params_any(this->mpPcmhandle, this->mpPcmParams);
    snd_pcm_hw_params_set_access(this->mpPcmhandle, this->mpPcmParams,  mpParams->uPcmAccess);
    snd_pcm_hw_params_set_format(this->mpPcmhandle, this->mpPcmParams, mpParams->uPcmFormat);
    snd_pcm_hw_params_set_channels(this->mpPcmhandle, this->mpPcmParams, mpParams->uPcmChannel);
    snd_pcm_hw_params_set_rate_near(this->mpPcmhandle,this->mpPcmParams, &(mpParams->uPcmSampleRate), 0);
    snd_pcm_hw_params_set_period_size_near(this->mpPcmhandle, this->mpPcmParams, &mFrameSize, 0);
    snd_pcm_hw_params(this->mpPcmhandle, this->mpPcmParams);
    snd_pcm_prepare(this->mpPcmhandle);
	 // 开始录音和编码
    pPcmBuffer = (INT16 *)SysMemory_malloc(mFrameSize*(mpParams->uPcmChannel)*sizeof(INT16));
}

// 初始化 FDK-AAC 编码器
INT32 MediaAudio::MediaAudioAacEncoder(PUINT8 pAsc,UINT uSize) {

    if (aacEncOpen(&mAacEncoder, 0, mpParams->uPcmChannel) != AACENC_OK) {
        LOG_ERROR("Failed to open AAC encoder\n");
        return ERROR;
    }

    // 设置编码参数
    aacEncoder_SetParam(mAacEncoder, AACENC_AOT, mpParams->stCodecParam.uAOT); // MPEG-4 AAC-LC
    aacEncoder_SetParam(mAacEncoder, AACENC_SAMPLERATE, mpParams->uPcmSampleRate);
    aacEncoder_SetParam(mAacEncoder, AACENC_CHANNELMODE, mpParams->uPcmChannel == 2 ? MODE_2 : MODE_1);
    aacEncoder_SetParam(mAacEncoder, AACENC_BITRATE, mpParams->stCodecParam.uBitRate);
    aacEncoder_SetParam(mAacEncoder, AACENC_TRANSMUX, mpParams->stCodecParam.uTransMux); // 输出 raw AAC（无 ADTS）

    if (aacEncEncode(mAacEncoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
        LOG_ERROR("Failed to initialize AAC encoder\n");
        return ERROR;
    }

    // 获取 AudioSpecificConfig（用于 MP4 的 esds box）
    if (aacEncGetAudioSpecificConfig(mAacEncoder, pAsc, &uSize) == AACENC_OK) {
        return OK;
        // 可保存 asc 用于后续 MP4 封装
    }

    return  OK;
}



INT32 MediaAudio::MediaAudioSendAccStraem(VOID)
{
	uint8_t *aac_data;
	int aac_len;
	if (snd_pcm_readi(this->mpPcmhandle, pPcmBuffer, mFrameSize) != mFrameSize) {
            snd_pcm_recover(this->mpPcmhandle, -EPIPE, 0);
     }
	 if (encode_pcm_to_aac(pPcmBuffer, mFrameSize * (mpParams->uPcmChannel), &aac_data, &aac_len) == 0) {
            //送流
     }
	 return OK;
}

