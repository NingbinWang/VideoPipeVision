#include "MediaAudioInner.h"
#include "SysMemory.h"
#include "MediaPriv.h"
#include "SysTime.h"

MediaAudio* MediaAudio::createNew(AUDIO_CFG_PARAM_T&pParams)
{
    return New<MediaAudio>::allocate(&pParams);
}
MediaAudio::MediaAudio(AUDIO_CFG_PARAM_T *pParams):
mpParams(pParams)
{

	 muChan = pParams->uChan;
	 mFrameSize = 1024;//pParams->uPcmFrameSize;
	 MediaAudioRecordInit(pParams->strDevname);
	 MediaAudioAacEncoder(&(pParams->stCodecParam.stEncoderInfo));
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
    snd_pcm_hw_params_set_access(this->mpPcmhandle, this->mpPcmParams, (snd_pcm_access_t)mpParams->uPcmAccess);
    snd_pcm_hw_params_set_format(this->mpPcmhandle, this->mpPcmParams, (snd_pcm_format_t)mpParams->uPcmFormat);
    snd_pcm_hw_params_set_channels(this->mpPcmhandle, this->mpPcmParams, mpParams->uPcmChannel);
    snd_pcm_hw_params_set_rate_near(this->mpPcmhandle,this->mpPcmParams, &(mpParams->uPcmSampleRate), 0);
    snd_pcm_hw_params_set_period_size_near(this->mpPcmhandle, this->mpPcmParams, &mFrameSize, 0);
	snd_pcm_uframes_t buffer_size = mFrameSize * 4; // Buffer = Period * 4
    if (snd_pcm_hw_params_set_buffer_size_near(this->mpPcmhandle, this->mpPcmParams, &buffer_size) < 0) {
        LOG_ERROR("Unable to set buffer size %ld", buffer_size);
        return ERROR;
    }
    snd_pcm_hw_params(this->mpPcmhandle, this->mpPcmParams);
    snd_pcm_prepare(this->mpPcmhandle);
	 // 开始录音和编码
    pPcmBuffer = (INT16 *)SysMemory_malloc(mFrameSize*(mpParams->uPcmChannel)*sizeof(INT16));
    return OK;
}

// 初始化 FDK-AAC 编码器
INT32 MediaAudio::MediaAudioAacEncoder(AACENC_INFO_T* pInfo) {

    if (aacEncOpen(&mAacEncoder, 0, mpParams->uPcmChannel) != AACENC_OK) {
        LOG_ERROR("Failed to open AAC encoder\n");
        return ERROR;
    }

    // 设置编码参数
    mpParams->uPcmSampleRate = 48000;
    // 2. 设置音频对象类型 (AAC-LC)
    // 注意：MODE_2 仅在双声道时有效
    if (aacEncoder_SetParam(mAacEncoder, AACENC_AOT, 2) != AACENC_OK) {
        LOG_ERROR("Failed to set AOT (AAC-LC)\n");
        return ERROR;
    }
    // 3. 设置采样率
    // 必须确保是标准采样率
    if (aacEncoder_SetParam(mAacEncoder, AACENC_SAMPLERATE, mpParams->uPcmSampleRate) != AACENC_OK) {
        LOG_ERROR("Failed to set SAMPLERATE: %d\n", mpParams->uPcmSampleRate);
        return ERROR;
    }

    // 4. 修复：动态设置声道模式 (核心修复点)
    // 不要硬编码 MODE_2，必须根据实际通道数判断
    if (mpParams->uPcmChannel == 2) {
        if (aacEncoder_SetParam(mAacEncoder, AACENC_CHANNELMODE, MODE_2) != AACENC_OK) {
            LOG_ERROR("Failed to set CHANNELMODE: MODE_2 (Stereo)\n");
            return ERROR;
        }
    } else if (mpParams->uPcmChannel == 1) {
        if (aacEncoder_SetParam(mAacEncoder, AACENC_CHANNELMODE, MODE_1) != AACENC_OK) {
            LOG_ERROR("Failed to set CHANNELMODE: MODE_1 (Mono)\n");
            return ERROR;
        }
    } else {
        LOG_ERROR("Unsupported channel count: %d\n", mpParams->uPcmChannel);
        return ERROR;
    }

    // 5. 设置通道顺序
    if (aacEncoder_SetParam(mAacEncoder, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        LOG_ERROR("Failed to set CHANNELORDER\n");
        return ERROR;
    }

    // 6. 关键修复：显式设置传输格式为 ADTS
    // 这是解决 Error 35 的常见原因
    if (aacEncoder_SetParam(mAacEncoder, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
        LOG_ERROR("Failed to set TRANSMUX (ADTS)\n");
        return ERROR;
    }

    AACENC_ERROR err = aacEncEncode(mAacEncoder, NULL, NULL, NULL, NULL);
	if (err != AACENC_OK) {
    	LOG_ERROR("aacEncEncode init failed with code: %d", err);
    	switch(err) {
        	case AACENC_INIT_ERROR: LOG_ERROR("Encoder init error - likely parameter mismatch \n"); break;
        	case AACENC_UNSUPPORTED_PARAMETER: LOG_ERROR("Unsupported parameter (rate/channels/bitrade)\n"); break;
        	case AACENC_MEMORY_ERROR: LOG_ERROR("Not enough memory\n"); break;
        	default: break;
    	}
    	return ERROR;
	}

    // 获取 AudioSpecificConfig（用于 MP4 的 esds box）
    if (aacEncInfo(mAacEncoder, (AACENC_InfoStruct*)pInfo) != AACENC_OK) {
        return ERROR;
        // 可保存 asc 用于后续 MP4 封装
    }

    return  OK;
}

// 编码一帧 PCM 数据
INT32 MediaAudio::MediaAudioEncPcmToAac(INT16 *pPcmSamples, UINT uNumSamples, UINT8 *pOutData)
{
	
    AACENC_BufDesc stInBuf;
    AACENC_BufDesc stOutBuf;
    AACENC_InArgs  stInArgs;
    AACENC_OutArgs stOutArgs;
    INT32          iOutLen = 0;
    UINT           uIndex = 0 ; 
    static uint8_t OutputBuffer[AUDIOBUFFERSIZE];
    INT_PCM inputBuffer[uNumSamples];
    for (uIndex = 0; uIndex < uNumSamples; uIndex++) {
        inputBuffer[uIndex] = pPcmSamples[uIndex];
    }
    // 输入配置
    void *in_ptr = inputBuffer;
    INT in_sizes = uNumSamples * sizeof(INT_PCM);
    INT in_ident = IN_AUDIO_DATA;
    stInBuf.numBufs = 1;
    stInBuf.bufs = &in_ptr;
    stInBuf.bufferIdentifiers = &in_ident;
    stInBuf.bufSizes = &in_sizes;
    stInBuf.bufElSizes = &in_sizes;
    // 输出缓冲区（需足够大）
    
    void *out_ptr = OutputBuffer;
    INT out_sizes = sizeof(OutputBuffer);
    INT out_ident = OUT_BITSTREAM_DATA;
    stOutBuf.numBufs = 1;
    stOutBuf.bufs = &out_ptr;
    stOutBuf.bufferIdentifiers = &out_ident;
    stOutBuf.bufSizes = &out_sizes;
    stOutBuf.bufElSizes = &out_sizes;
    stInArgs.numInSamples = uNumSamples;
    AACENC_ERROR err = aacEncEncode(mAacEncoder, &stInBuf, &stOutBuf, &stInArgs, &stOutArgs);
    if (err != AACENC_OK && err != AACENC_ENCODE_EOF) {
        LOG_ERROR("Encoding error: %d\n", err);
        return ERROR;
    }

    if (stOutArgs.numOutBytes > 0) {
        iOutLen = stOutArgs.numOutBytes;
        SysMemory_copy((VOID *)pOutData, (VOID *)OutputBuffer, (UINT32)iOutLen);
        return iOutLen;
    }
    return ERROR;
}


INT32 MediaAudio::MediaAudioGetAacData(UINT8* pAacdata)
{
	snd_pcm_sframes_t size = (snd_pcm_sframes_t)mFrameSize;
	int ret = snd_pcm_readi(mpPcmhandle, (void *)pPcmBuffer, mFrameSize);
	if(ret != size) {
            // 发生 overrun，尝试恢复
        if (snd_pcm_recover(this->mpPcmhandle, ret, 0) == 0) {
            // 恢复成功，但这次的数据丢了，返回 0 表示“没有有效编码数据，但不要报错”
            // 或者你可以尝试重新读取一次
            LOG_DEBUG("ALSA Overrun recovered, dropping one frame\n");
            return 0; // 修改点1：返回0而不是ERROR，避免上层报错
        } else {
            // 恢复失败，可能是严重错误
            LOG_ERROR("ALSA read/write error: %s\n", snd_strerror(ret));
            return ERROR;
        }
     }
	return MediaAudioEncPcmToAac((INT16*)pPcmBuffer, mFrameSize * (mpParams->uPcmChannel), pAacdata);
}


INT32 MediaAudio::MediaAudioSendStream(MEDIA_AUDIO_INFO_T* pstAudioInfo)
{
 	MEDIA_INNER_PARAM_T* pInnerParam =  Media_Get_InnerParam();
 	UINT32 uFrameSize = 0;
 	CHAR *pOutputbuf = (CHAR*)SysMemory_malloc(AUDIOBUFFERSIZE*2);
 	pstAudioInfo->eType = mpParams->stCodecParam.eType;
 	pstAudioInfo->u32AudioNum = muChan;
 	pstAudioInfo->u32SampleRate = mpParams->uPcmSampleRate;
 	if(pstAudioInfo->pAddr != NULL){
 		pstAudioInfo->iFrameLen = MediaAudioGetAacData(pstAudioInfo->pAddr);
 	}else{
		SysMemory_free(pOutputbuf);
		return ERROR;
 	}
	 if(pstAudioInfo->iFrameLen <= 0) { 
        // 如果返回0或负数，不打印错误，直接返回 OK，让循环继续
        // LOG_DEBUG 这一行可以保留，但不要认为是严重错误
        return ERROR; // 修改点2：这里返回 OK，防止外部逻辑中断
    }
 	SysMemory_copy((VOID *)pOutputbuf, (VOID *)pstAudioInfo, sizeof(MEDIA_AUDIO_INFO_T));
 	SysMemory_copy((VOID *)(pOutputbuf+sizeof(MEDIA_AUDIO_INFO_T)), (VOID *)pstAudioInfo->pAddr, sizeof(MEDIA_AUDIO_INFO_T));
 	uFrameSize = pstAudioInfo->iFrameLen+sizeof(MEDIA_AUDIO_INFO_T);
 	pInnerParam->pStream->SendStreamToAudioPool((PUINT8)pOutputbuf,uFrameSize,0);
 	SysMemory_free(pOutputbuf);
	return OK;
 	
 	
}


