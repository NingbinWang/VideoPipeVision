#ifndef _MEDIA_DEC_H_
#define _MEDIA_DEC_H_
#include "MediaConfig.h"
#include "MediaFrame.h"
#include "SysMutex.h"
#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

typedef struct
{
    volatile UINT32   status;                                 /*当前状态*/
    volatile UINT32   bufUsage;                               /*缓冲使用率*/
    volatile UINT32   receivedData;                           /*接收到的数据*/
    volatile UINT32   invlidData;                             /*无效的数据*/
    volatile UINT32   decodedV;                               /*解码的视频帧*/
    volatile UINT32   decodedA;                               /*解码的音频帧*/
    volatile UINT32   decPassedV;                             /*解码跳过的视频帧*/
    volatile UINT32   decPassedA;                             /*解码跳过的音频帧*/
    volatile UINT32   decFirstPts;                            /*解码第一帧的时间戳*/
    volatile UINT32   curDispPts;                             /*当前显示的时间戳*/
    volatile UINT32   decFrmNumCnt;                           /*已经解码的帧数*/
    volatile UINT32   fpsDecV;                                /*视频解码帧率*/
    volatile UINT32   fpsDecA;                                /*音频解码帧率*/
    volatile UINT32   fpsDispA;                               /*音频输出帧率*/
    volatile UINT32   imgW;                                   /*解码器当前的图像大小*/
    volatile UINT32   imgH;       
    volatile UINT32   fileIdx;                                /*当前的文件索引*/
    volatile UINT32   lastIPos;                               /*当前的文件索引*/
    volatile UINT32   frmNum;                                 /*最后显示的帧号*/
    volatile UINT32   frmTime;                                /*当前的解码器时间*/
    volatile UINT32   absTime;                                /*当前绝对时间*/
	UINT8			  res[4];
}DEC_STATUS_T;

/*解码码流共享缓存接口定义*/
typedef struct 
{
	MUTEX_ID                 mDevPool;
    void *                	 addr[MAX_SHARE_ADDR];   /*多核/多进程访问地址 */
    UINT32                   totalLen;       		/*解码缓冲长度*/
    UINT32                   wIdx;           		/*解码缓冲写索引*/
    UINT32                   rIdx;           		/*解码缓冲读索引*/
	UINT8		             res[4];
}DEC_SHARE_BUF_T;


#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif