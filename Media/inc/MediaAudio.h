#ifndef _MEDIAAUDIO_H_
#define _MEDIAAUDIO_H_

#include "MediaConfig.h"
#include "MediaFrame.h"

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/


#define AUDIO_DEVNAME_STR_LEN (16)

/**
 *  Provides some info about the encoder configuration.
 */
typedef struct {
    UINT maxOutBufBytes;
    UINT maxAncBytes; 
    UINT inBufFillLevel; 
    UINT inputChannels;
    UINT frameLength; 
    UINT nDelay; 
    UINT nDelayCore; 
    UINT8 confBuf[64];
    UINT confSize; 
} AACENC_INFO_T;


//缓冲区的大小
typedef struct
{
	MUTEX_ID              mAudPool;
    void*    			  addr[MAX_SHARE_ADDR];        //多核/多进程访问地址 0为虚拟地址 1为物理地址
    UINT32    	          totalLen;      //缓冲区长度
    UINT32                rIdx;        //缓冲写索引
    UINT32                wIdx;        //缓冲读索引
}AUDIO_POOL_INFO_T;



typedef struct
{
	MEDIA_AUDIO_CODEC_TYPE_E eType;
	UINT               		uBitRate;
	UINT               		uTransMux;
	UINT               		uAOT;
	AACENC_INFO_T      		stEncoderInfo;
}AUDIO_CODEC_PARAM_T;



typedef struct
{
	UINT    			uChan;
	CHAR    			strDevname[AUDIO_DEVNAME_STR_LEN];    //设备节点名
	UINT    			uPcmChannel;
	UINT    			uPcmSampleRate;
	UINT    			uPcmAccess;
	UINT    			uPcmFormat;
	UINT    			uPcmFrameSize;
    AUDIO_CODEC_PARAM_T stCodecParam;
    /*audio codec type*/
    UINT32 res[3];
    /**< 预留*/
}AUDIO_CFG_PARAM_T;


#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif
