#ifndef _MEDIAAUDIO_H_
#define _MEDIAAUDIO_H_

#include "MediaConfig.h"
#include "MediaFrame.h"

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/
//缓冲区的大小
typedef struct
{
    void*    			  addr[MAX_SHARE_ADDR];        //多核/多进程访问地址 0为虚拟地址 1为物理地址
    UINT32    	          u32len;      //缓冲区长度
    UINT32                rIdx;        //缓冲写索引
    UINT32                wIdx;        //缓冲读索引
}AUDIO_POOL_INFO_T;

typedef enum 
{
	AUCODEC_MAX,
} AUDIO_CODEC_TYPE_E;

typedef struct
{
    AUDIO_CODEC_TYPE_E aCodecType;
    /*audio codec type*/
    UINT32 res[3];
    /**< 预留*/
}AUDIO_CFG_PARAM_T;


#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif
