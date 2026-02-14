#include "MediaAudioInner.h"
#include "SysMemory.h"
#include "MediaPriv.h"
#include "SysTime.h"
#include "SysPthread.h"
#include <sys/sysinfo.h>

MediaAudio* MediaAudio::createNew(AUDIO_CFG_PARAM_T&pParams)
{
    return New<MediaAudio>::allocate(&pParams);
}

MediaAudio::MediaAudio(AUDIO_CFG_PARAM_T *pParams):
mpParams(pParams)
{
    muChan = pParams->uChan;
    
    // 初始化成员变量
    mFrameSize = 256;
    mpPcmhandle = nullptr;
    mpPcmParams = nullptr;
    pPcmBuffer = nullptr;
    mAacEncoder = nullptr;
    
    // 首先初始化编码器
    MediaAudioAacEncoder(&(pParams->stCodecParam.stEncoderInfo));
    
    // 然后初始化录音
    MediaAudioRecordInit(pParams->strDevname);
    
    SysMutex_create(&this->mtxAudio, 0);
}

MediaAudio::~MediaAudio() 
{
    // 清理顺序很重要
    SysMutex_destroy(&this->mtxAudio);
    
    if(pPcmBuffer){
        SysMemory_free(pPcmBuffer);
        pPcmBuffer = nullptr;
    }
    
    if(this->mpPcmParams) {
        snd_pcm_hw_params_free(this->mpPcmParams);
        this->mpPcmParams = nullptr;
    }
    
    if(this->mpPcmhandle){
        snd_pcm_close(this->mpPcmhandle);
        this->mpPcmhandle = nullptr;
    }
    
    if(mAacEncoder){
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
    }
}

// 检查系统可用内存
static long get_available_memory_kb() {
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        return si.freeram * si.mem_unit / 1024; // 返回KB
    }
    return -1;
}

INT32 MediaAudio::MediaAudioRecordInit(const char* strDevName)
{    
    LOG_DEBUG("Starting audio device initialization: %s\n", strDevName);
    
    // 检查系统内存
    long free_mem_kb = get_available_memory_kb();
    LOG_DEBUG("Available memory: %ld KB\n", free_mem_kb);
    
    // 关闭旧句柄
    if (this->mpPcmhandle) {
        snd_pcm_close(this->mpPcmhandle);
        this->mpPcmhandle = nullptr;
    }
    
    // 释放旧的硬件参数结构
    if (this->mpPcmParams) {
        snd_pcm_hw_params_free(this->mpPcmParams);
        this->mpPcmParams = nullptr;
    }
    
    // 尝试打开设备
    snd_pcm_t *pcm_handle;
    int err = snd_pcm_open(&pcm_handle, strDevName, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        LOG_ERROR("Cannot open audio device %s: %s\n", strDevName, snd_strerror(err));
        return ERROR;
    }
    
    LOG_DEBUG("Successfully opened audio device: %s\n", strDevName);
    
    // 分配硬件参数结构
    snd_pcm_hw_params_t *hw_params;
    err = snd_pcm_hw_params_malloc(&hw_params);
    if (err < 0) {
        LOG_ERROR("Cannot allocate hardware parameters: %s\n", snd_strerror(err));
        snd_pcm_close(pcm_handle);
        return ERROR;
    }
    
    // 初始化硬件参数
    snd_pcm_hw_params_any(pcm_handle, hw_params);
    
    // 设置访问类型
    err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, (snd_pcm_access_t)mpParams->uPcmAccess);
    if (err < 0) {
        LOG_ERROR("Cannot set access type: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(pcm_handle);
        return ERROR;
    }
    
    // 设置格式
    err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, (snd_pcm_format_t)mpParams->uPcmFormat);
    if (err < 0) {
        LOG_ERROR("Cannot set sample format: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(pcm_handle);
        return ERROR;
    }
    
    // 设置通道数
    err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, mpParams->uPcmChannel);
    if (err < 0) {
        LOG_ERROR("Cannot set channel count: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(pcm_handle);
        return ERROR;
    }
    
    // 设置采样率
    unsigned int requested_rate = mpParams->uPcmSampleRate;
    err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &requested_rate, 0);
    if (err < 0) {
        LOG_ERROR("Cannot set sample rate: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(pcm_handle);
        return ERROR;
    }
    
    // 内存优化：使用极小的周期和缓冲区
    snd_pcm_uframes_t requested_period_size = 128; // 初始请求
    err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &requested_period_size, 0);
    if (err < 0) {
        LOG_DEBUG("Trying smaller period size: %s\n", snd_strerror(err));
        requested_period_size = 64; // 进一步减小
        err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &requested_period_size, 0);
        if (err < 0) {
            LOG_ERROR("Cannot set any period size: %s\n", snd_strerror(err));
            snd_pcm_hw_params_free(hw_params);
            snd_pcm_close(pcm_handle);
            return ERROR;
        }
    }
    
    // 设置极小缓冲区
    snd_pcm_uframes_t requested_buffer_size = requested_period_size * 2; // 只使用2个周期
    if (requested_buffer_size > 512) requested_buffer_size = 512; // 严格限制
    
    err = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params, &requested_buffer_size);
    if (err < 0) {
        LOG_ERROR("Cannot set buffer size: %s\n", snd_strerror(err));
        snd_pcm_hw_params_free(hw_params);
        snd_pcm_close(pcm_handle);
        return ERROR;
    }
    
    // 应用硬件参数
    err = snd_pcm_hw_params(pcm_handle, hw_params);
    if (err < 0) {
        LOG_ERROR("Cannot apply hardware parameters for audio device %s: %s\n", strDevName, snd_strerror(err));
        
        // 尝试使用更保守的参数配置
        LOG_DEBUG("Attempting conservative configuration...\n");
        
        // 释放之前的参数结构
        snd_pcm_hw_params_free(hw_params);
        
        // 重新分配参数结构
        err = snd_pcm_hw_params_malloc(&hw_params);
        if (err < 0) {
            LOG_ERROR("Cannot reallocate hardware parameters: %s\n", snd_strerror(err));
            snd_pcm_close(pcm_handle);
            return ERROR;
        }
        
        snd_pcm_hw_params_any(pcm_handle, hw_params);
        
        // 设置基本参数
        snd_pcm_hw_params_set_access(pcm_handle, hw_params, (snd_pcm_access_t)mpParams->uPcmAccess);
        snd_pcm_hw_params_set_format(pcm_handle, hw_params, (snd_pcm_format_t)mpParams->uPcmFormat);
        snd_pcm_hw_params_set_channels(pcm_handle, hw_params, mpParams->uPcmChannel);
        
        // 使用固定的采样率
        unsigned int fixed_rate = 48000;
        snd_pcm_hw_params_set_rate(pcm_handle, hw_params, fixed_rate, 0);
        
        // 使用最小可能的参数
        snd_pcm_uframes_t small_period = 64;
        snd_pcm_hw_params_set_period_size(pcm_handle, hw_params, small_period, 0);
        snd_pcm_uframes_t small_buffer = 128;
        snd_pcm_hw_params_set_buffer_size(pcm_handle, hw_params, small_buffer);
        
        err = snd_pcm_hw_params(pcm_handle, hw_params);
        if (err < 0) {
            LOG_ERROR("Conservative configuration also failed: %s\n", snd_strerror(err));
            snd_pcm_hw_params_free(hw_params);
            snd_pcm_close(pcm_handle);
            return ERROR;
        }
    }
    
    // 保存成功的配置
    this->mpPcmhandle = pcm_handle;
    this->mpPcmParams = hw_params;
    
    // 应用参数后立即获取实际配置参数 - 关键修复：确保参数已应用
    unsigned int actual_rate = mpParams->uPcmSampleRate;  // 默认值
    unsigned int actual_channels = mpParams->uPcmChannel; // 默认值
    snd_pcm_uframes_t actual_period_size = requested_period_size; // 默认值
    snd_pcm_uframes_t actual_buffer_size = requested_buffer_size; // 默认值
    
    // 尝试获取实际配置参数
    err = snd_pcm_hw_params_get_rate(hw_params, &actual_rate, NULL);
    if (err < 0) {
        LOG_ERROR("Cannot get sample rate: %s\n", snd_strerror(err));
        actual_rate = mpParams->uPcmSampleRate; // 使用原始值
    }
    
    err = snd_pcm_hw_params_get_channels(hw_params, &actual_channels);
    if (err < 0) {
        LOG_ERROR("Cannot get channels: %s\n", snd_strerror(err));
        actual_channels = mpParams->uPcmChannel; // 使用原始值
    }
    
    err = snd_pcm_hw_params_get_period_size(hw_params, &actual_period_size, NULL);
    if (err < 0) {
        LOG_ERROR("Cannot get period size: %s\n", snd_strerror(err));
        actual_period_size = requested_period_size; // 使用请求值
    }
    
    err = snd_pcm_hw_params_get_buffer_size(hw_params, &actual_buffer_size); // 正确的函数调用
    if (err < 0) {
        LOG_ERROR("Cannot get buffer size: %s\n", snd_strerror(err));
        actual_buffer_size = requested_buffer_size; // 使用请求值
    }
    
    LOG_DEBUG("Hardware parameters configured: channels=%u, sample_rate=%u, period_size=%lu, buffer_size=%lu\n", 
              actual_channels, actual_rate, (unsigned long)actual_period_size, (unsigned long)actual_buffer_size);
    
    // 验证参数不为零
    if (actual_channels == 0) {
        LOG_ERROR("Channels is zero, using fallback value: %d\n", mpParams->uPcmChannel);
        actual_channels = mpParams->uPcmChannel ? mpParams->uPcmChannel : 2;
    }
    
    if (actual_rate == 0) {
        LOG_ERROR("Sample rate is zero, using fallback value: %d\n", mpParams->uPcmSampleRate);
        actual_rate = mpParams->uPcmSampleRate ? mpParams->uPcmSampleRate : 48000;
    }
    
    if (actual_period_size == 0) {
        LOG_ERROR("Period size is zero, using fallback value: %d\n", requested_period_size);
        actual_period_size = requested_period_size ? requested_period_size : 128;
    }
    
    if (actual_buffer_size == 0) {
        LOG_ERROR("Buffer size is zero, using fallback value: %d\n", requested_buffer_size);
        actual_buffer_size = requested_buffer_size ? requested_buffer_size : 256;
    }
    
    // 更新参数
    mpParams->uPcmChannel = actual_channels;
    mpParams->uPcmSampleRate = actual_rate;
    mFrameSize = actual_period_size; // 使用实际的周期大小
    
    // 设置软件参数
    snd_pcm_sw_params_t *sw_params;
    err = snd_pcm_sw_params_malloc(&sw_params);
    if (err < 0) {
        LOG_ERROR("Cannot allocate software parameters: %s\n", snd_strerror(err));
        return ERROR;
    }
    
    snd_pcm_sw_params_current(pcm_handle, sw_params);
    
    err = snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, actual_period_size);
    if (err < 0) {
        LOG_ERROR("Cannot set avail min: %s\n", snd_strerror(err));
        snd_pcm_sw_params_free(sw_params);
        return ERROR;
    }
    
    err = snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, actual_period_size);
    if (err < 0) {
        LOG_ERROR("Cannot set start threshold: %s\n", snd_strerror(err));
        snd_pcm_sw_params_free(sw_params);
        return ERROR;
    }
    
    err = snd_pcm_sw_params(pcm_handle, sw_params);
    if (err < 0) {
        LOG_ERROR("Cannot apply software parameters: %s\n", snd_strerror(err));
        snd_pcm_sw_params_free(sw_params);
        return ERROR;
    }
    
    snd_pcm_sw_params_free(sw_params);
    snd_pcm_prepare(pcm_handle);
    
    // 计算PCM缓冲区大小 - 使用验证后的值
    size_t buffer_samples = actual_period_size * actual_channels;
    size_t required_memory = buffer_samples * sizeof(INT16);
    
    // 防止零大小分配
    if (required_memory == 0) {
        LOG_ERROR("Calculated zero buffer size! period_size=%lu, channels=%u\n", 
                  (unsigned long)actual_period_size, actual_channels);
        required_memory = 256 * 2 * sizeof(INT16); // 安全默认值
    }
    
    LOG_DEBUG("Required PCM buffer size: %zu bytes (%zu samples)\n", required_memory, buffer_samples);
    
    // 检查内存是否足够
    if (free_mem_kb > 0) {
        size_t required_kb = required_memory / 1024;
        if (required_kb > (size_t)(free_mem_kb / 4)) { // 要求4倍余量
            LOG_ERROR("Insufficient memory for PCM buffer. Required: %zu KB, Available: %ld KB\n", 
                      required_kb, free_mem_kb);
        }
    }
    
    // 释放旧缓冲区
    if (pPcmBuffer) {
        SysMemory_free(pPcmBuffer);
        pPcmBuffer = nullptr;
    }
    
    // 分配新缓冲区
    pPcmBuffer = (INT16 *)SysMemory_malloc(required_memory);
    if (!pPcmBuffer) {
        LOG_ERROR("Failed to allocate PCM buffer of size %zu\n", required_memory);
        snd_pcm_close(pcm_handle);
        this->mpPcmhandle = nullptr;
        return ERROR;
    }
    
    LOG_DEBUG("Successfully allocated PCM buffer of size %zu\n", required_memory);
    
    return OK;
}

// 初始化 FDK-AAC 编码器 - 优化版本
INT32 MediaAudio::MediaAudioAacEncoder(AACENC_INFO_T* pInfo) {
    // 先关闭旧编码器
    if (mAacEncoder) {
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
    }

    // 检查系统内存
    long free_mem_kb = get_available_memory_kb();
    LOG_DEBUG("Memory before AAC encoder init: %ld KB\n", free_mem_kb);

    // 创建编码器
    if (aacEncOpen(&mAacEncoder, 0, mpParams->uPcmChannel) != AACENC_OK) {
        LOG_ERROR("Failed to open AAC encoder\n");
        return ERROR;
    }

    // 设置编码参数
    // 使用RK356X友好的参数
    if (aacEncoder_SetParam(mAacEncoder, AACENC_AOT, 2) != AACENC_OK) { // AAC-LC
        LOG_ERROR("Failed to set AOT (AAC-LC)\n");
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
        return ERROR;
    }

    // 设置采样率 - 确保是支持的标准值
    if (mpParams->uPcmSampleRate != 48000 && mpParams->uPcmSampleRate != 44100) {
        mpParams->uPcmSampleRate = 48000; // 强制使用48kHz
    }
    
    if (aacEncoder_SetParam(mAacEncoder, AACENC_SAMPLERATE, mpParams->uPcmSampleRate) != AACENC_OK) {
        LOG_ERROR("Failed to set SAMPLERATE: %d\n", mpParams->uPcmSampleRate);
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
        return ERROR;
    }

    // 根据实际通道数设置声道模式
    if (mpParams->uPcmChannel == 2) {
        if (aacEncoder_SetParam(mAacEncoder, AACENC_CHANNELMODE, MODE_2) != AACENC_OK) {
            LOG_ERROR("Failed to set CHANNELMODE: MODE_2 (Stereo)\n");
            aacEncClose(&mAacEncoder);
            mAacEncoder = nullptr;
            return ERROR;
        }
    } else if (mpParams->uPcmChannel == 1) {
        if (aacEncoder_SetParam(mAacEncoder, AACENC_CHANNELMODE, MODE_1) != AACENC_OK) {
            LOG_ERROR("Failed to set CHANNELMODE: MODE_1 (Mono)\n");
            aacEncClose(&mAacEncoder);
            mAacEncoder = nullptr;
            return ERROR;
        }
    } else {
        LOG_ERROR("Unsupported channel count: %d\n", mpParams->uPcmChannel);
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
        return ERROR;
    }

    // 设置传输格式为ADTS
    if (aacEncoder_SetParam(mAacEncoder, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
        LOG_ERROR("Failed to set TRANSMUX (ADTS)\n");
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
        return ERROR;
    }

    // 初始化编码器
    AACENC_ERROR err = aacEncEncode(mAacEncoder, NULL, NULL, NULL, NULL);
    if (err != AACENC_OK) {
        LOG_ERROR("aacEncEncode init failed with code: %d", err);
        switch(err) {
            case AACENC_INIT_ERROR: 
                LOG_ERROR("Encoder init error - likely parameter mismatch\n"); 
                break;
            case AACENC_UNSUPPORTED_PARAMETER: 
                LOG_ERROR("Unsupported parameter (rate/channels/bitrade)\n"); 
                break;
            case AACENC_MEMORY_ERROR: 
                LOG_ERROR("Not enough memory for AAC encoder\n"); 
                break;
            default: 
                break;
        }
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
        return ERROR;
    }

    // 获取编码器信息
    if (aacEncInfo(mAacEncoder, (AACENC_InfoStruct*)pInfo) != AACENC_OK) {
        LOG_ERROR("Failed to get AAC encoder info\n");
        aacEncClose(&mAacEncoder);
        mAacEncoder = nullptr;
        return ERROR;
    }

    LOG_DEBUG("AAC encoder initialized successfully\n");
    return OK;
}

// 编码一帧 PCM 数据 - 优化版本
INT32 MediaAudio::MediaAudioEncPcmToAac(INT16 *pPcmSamples, UINT uNumSamples, UINT8 *pOutData)
{
    if (!pPcmSamples || !pOutData || !mAacEncoder) {
        LOG_ERROR("Invalid input parameters\n");
        return ERROR;
    }
    
    static uint8_t OutputBuffer[AUDIOBUFFERSIZE];
    INT_PCM inputBuffer[uNumSamples];
    
    // 转换样本格式
    for (UINT uIndex = 0; uIndex < uNumSamples; uIndex++) {
        inputBuffer[uIndex] = pPcmSamples[uIndex];
    }
    
    // 输入配置
    void *in_ptr = inputBuffer;
    INT in_sizes = uNumSamples * sizeof(INT_PCM);
    INT in_ident = IN_AUDIO_DATA;
    
    AACENC_BufDesc stInBuf = {0};
    stInBuf.numBufs = 1;
    stInBuf.bufs = &in_ptr;
    stInBuf.bufferIdentifiers = &in_ident;
    stInBuf.bufSizes = &in_sizes;
    stInBuf.bufElSizes = &in_sizes;
    
    // 输出缓冲区
    void *out_ptr = OutputBuffer;
    INT out_sizes = sizeof(OutputBuffer);
    INT out_ident = OUT_BITSTREAM_DATA;
    
    AACENC_BufDesc stOutBuf = {0};
    stOutBuf.numBufs = 1;
    stOutBuf.bufs = &out_ptr;
    stOutBuf.bufferIdentifiers = &out_ident;
    stOutBuf.bufSizes = &out_sizes;
    stOutBuf.bufElSizes = &out_sizes;
    
    AACENC_InArgs stInArgs = {0};
    stInArgs.numInSamples = uNumSamples;
    
    AACENC_OutArgs stOutArgs = {0};
    
    AACENC_ERROR err = aacEncEncode(mAacEncoder, &stInBuf, &stOutBuf, &stInArgs, &stOutArgs);
    if (err != AACENC_OK && err != AACENC_ENCODE_EOF) {
        LOG_ERROR("Encoding error: %d\n", err);
        return ERROR;
    }

    if (stOutArgs.numOutBytes > 0) {
        INT32 iOutLen = stOutArgs.numOutBytes;
        SysMemory_copy((VOID *)pOutData, (VOID *)OutputBuffer, (UINT32)iOutLen);
        return iOutLen;
    }
    
    return 0; // 没有输出数据但不是错误
}

INT32 MediaAudio::MediaAudioGetAacData(UINT8* pAacdata)
{
    if (!pAacdata || !mpPcmhandle || !pPcmBuffer) {
        LOG_ERROR("Invalid parameters\n");
        return ERROR;
    }
    
    // 修复：使用类型转换避免符号比较警告
    snd_pcm_sframes_t ret = snd_pcm_readi(mpPcmhandle, (void *)pPcmBuffer, mFrameSize);
    
    if (ret < 0) {
        LOG_DEBUG("ALSA read error: %s\n", snd_strerror(ret));
        
        // 尝试恢复
        int recover_result = snd_pcm_recover(mpPcmhandle, ret, 0);
        if (recover_result < 0) {
            LOG_ERROR("ALSA recovery failed: %s\n", snd_strerror(ret));
            return ERROR;
        }
        
        // 重新读取
        ret = snd_pcm_readi(mpPcmhandle, (void *)pPcmBuffer, mFrameSize);
        if (ret < 0) {
            LOG_ERROR("Read failed after recovery: %s\n", snd_strerror(ret));
            return ERROR;
        }
    }
    
    // 修复：使用类型转换避免符号比较警告
    if ((snd_pcm_uframes_t)ret != mFrameSize) {
        LOG_DEBUG("Read returned %ld frames, expected %u\n", (long)ret, (unsigned)mFrameSize);
    }
    
    return MediaAudioEncPcmToAac((INT16*)pPcmBuffer, mFrameSize * (mpParams->uPcmChannel), pAacdata);
}

INT32 MediaAudio::MediaAudioSendStream(MEDIA_AUDIO_INFO_T* pstAudioInfo)
{
    if (!pstAudioInfo) {
        LOG_ERROR("Invalid input parameter\n");
        return ERROR;
    }
    
    MEDIA_INNER_PARAM_T* pInnerParam = Media_Get_InnerParam();
    if (!pInnerParam || !pInnerParam->pStream) {
        LOG_ERROR("Invalid inner param or stream\n");
        return ERROR;
    }
    
    UINT32 uFrameSize = 0;
    CHAR *pOutputbuf = (CHAR*)SysMemory_malloc(AUDIOBUFFERSIZE * 2);
    if (!pOutputbuf) {
        LOG_ERROR("Failed to allocate output buffer\n");
        return ERROR;
    }
    
    pstAudioInfo->eType = mpParams->stCodecParam.eType;
    pstAudioInfo->u32AudioNum = muChan;
    pstAudioInfo->u32SampleRate = mpParams->uPcmSampleRate;
    
    if (pstAudioInfo->pAddr != NULL) {
        pstAudioInfo->iFrameLen = MediaAudioGetAacData(pstAudioInfo->pAddr);
    } else {
        SysMemory_free(pOutputbuf);
        return ERROR;
    }
    
    if (pstAudioInfo->iFrameLen <= 0) {
        SysMemory_free(pOutputbuf);
        return OK; // 不是错误，只是没有数据
    }
    
    // 复制头部信息
    SysMemory_copy((VOID *)pOutputbuf, (VOID *)pstAudioInfo, sizeof(MEDIA_AUDIO_INFO_T));
    
    // 复制音频数据
    SysMemory_copy((VOID *)(pOutputbuf + sizeof(MEDIA_AUDIO_INFO_T)), 
                   (VOID *)pstAudioInfo->pAddr, pstAudioInfo->iFrameLen);
    
    uFrameSize = pstAudioInfo->iFrameLen + sizeof(MEDIA_AUDIO_INFO_T);
    
    // 发送到音频池
    pInnerParam->pStream->SendStreamToAudioPool((PUINT8)pOutputbuf, uFrameSize, 0);
    
    SysMemory_free(pOutputbuf);
    return OK;
}

INT32 MediaAudio::MediaAudioStartThread(VOID)
{
    if (SysPthread_create(NULL, (CHAR *)"AudioCodec", TASK_PRIORITY_0, 64*1024, (FUNCPTR)AudioCodecLoop, 0) != 0) {
        LOG_ERROR("Failed to create audio codec thread\n");
        return ERROR;
    }
    return OK;
}
