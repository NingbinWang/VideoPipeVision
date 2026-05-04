#ifndef _MEDIAPS_H_
#define _MEDIAPS_H_
#include "Common.h"
#include "MediaRefer.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
* 宏声明
******************************************************************************/
#define STUFF_BYTE_0XFF         0xff
#define PROGRAM_STREAM_PSH_TAG  0xba
#define PROGRAM_STREAM_PSM_TAG  0xbc
#define PES_VIDEO_BASE          0xe0
#define PES_AUDIO_BASE          0xc0
#define PES_PRIVT_BASE          0xdb
#define MAX_PES_PACKET_LEN      65496
#define MIN_PES_HEAD_STUFF_LEN	2
#define PS_PRIVATE_HEAD_LEN     12 



#define MEDIAPS_ERR_PARA_NULL	0x10000	
#define MEDIAPS_ERR_MEM_OVER	0x10001
#define MEDIAPS_ERR_STREAM_TYPE 0x10002	

/******************************************************************************
* 结构体声明
******************************************************************************/

typedef struct
{
	unsigned int	uStreamid;
	unsigned int	uAddpts;
	unsigned int	uTotalDataLen;
	unsigned int	uRestDataLen;
	unsigned int	uPayloadLen;
} MEDIAPS_PES_PACK_INFO_T;

typedef struct
{
	unsigned int	uVideoStreamid;
	unsigned int	uAudioStreamid;
	unsigned int	uPrivtStreamid;
	unsigned int	uStreamMode;		 /* 输入流模式*/
	unsigned int	uMaxByteRate;		 /* 码率，以byte为单位*/
	unsigned int	uMaxPesLen;		     /* 最大pes长度*/
	unsigned int	uVideoClip;
	unsigned int	uDscptSets;
	unsigned int	uBframeAudioSetPsh;
	unsigned int	uSetFrameEndFlg;

	unsigned int	uVideoStreamType;	/* 输入视频流类型 */
	unsigned int	uAudioStreamType;	/* 输入音频流类型 */
	unsigned int	uPrivtStreamType;	/* 输入私有流类型 */

	MEDIAPS_PES_PACK_INFO_T	stPesCur;
	
	//描述子内存空间
	unsigned char	strDeviceDsc[DEVICE_DESCRIPTOR_LEN];
	unsigned char	strVideoDsc[VIDEO_DESCRIPTOR_LEN];
	unsigned char	strAudioDsc[AUDIO_DESCRIPTOR_LEN];
	unsigned char	strVideoClipDsc[VIDEO_CLIP_DESCRIPTOR_LEN];
	unsigned char	strTimingHrdDsc[TIMING_HRD_DESCRIPTOR_LEN];
	unsigned char	PrivteHeader[PS_PRIVATE_HEAD_LEN];

}  MEDIAPS_PS_INFO_T;

typedef struct
{
    unsigned int	uStreamMode;        /* 输入流模式*/
	unsigned int	uMaxByteRate;	    /* 码率，以byte为单位*/
    unsigned int    uMaxPacketLen;     /* 最大 pes 包长度 */
    unsigned int	uVideoStreamType;  /* 输入视频流类型 */
    unsigned int	uAudioStreamType;  /* 输入音频流类型 */
	unsigned int	uPrivtStreamType;	/* 输入私有流类型 */
	unsigned int    uDscptSets;       /* 各种descriptor的设置开关，例如要添加video descriptor和audio_descriptor,则置为INCLUDE_VIDEO_DESCRIPTOR | INCLUDE_AUDIO_DESCRIPTOR*/
	unsigned int    uBframeAudioSetPsh; /* B帧和音频帧前是否添加PSH, 1为是，0为否 */
	unsigned int    uSetFrameEndFlg;    /* 是否在pes包头的填充字节里设置帧或nalu结束开始标记 */

    unsigned int    uRes[1];             /* 预留，兼容64位平台*/

    MEDIAREFER_STREAM_INFO stStreamInfo; 
} MEDIAPS_ES_INFO_T;


/* 复合器参数 */
typedef struct
{
    unsigned int        uBufferSize;
    unsigned int        uRes[1];   /* 预留，兼容64位平台*/ 
    unsigned char*      pBuffer;
    MEDIAPS_ES_INFO_T   stInfo;
} MEDIAPS_COMPLEX_PARAM_T;

/* 数据块处理参数 */
typedef struct
{
    unsigned int	uFrameType;         /* 输入帧类型                           */
    unsigned int    uIsFirstUnit;      /* 是否是一帧的第一个unit。标准H.264每帧会分成多个unit*/
                                        /* 其余编码每帧都只有一个unit */
	unsigned int	uIsLastUnit;		/* 是否是一帧的最后一个unit		*/
    unsigned int    uIsKeyFrame;       /* 是否关键数据(I帧)                    */
	unsigned int    uIsUnitStart;      /* 若是一个nalu或一帧的第一段数据，则置1，若送进的是完整的一帧或一个nalu也置1*/
	unsigned int    uIsUnitEnd;        /* 若是一个nalu或一帧的最后一段数据，则置1，若送进的是完整的一帧或一个nalu也置1*/
    unsigned int    uSysClkRef;        /* 系统参考时钟，以 1/45000 秒为单位    */
    unsigned int    uTimeStamp;        /* 该帧在接收端的显示时标，单位同上     */
	unsigned int	uFrameNum;			/* 当前帧号		  */
    unsigned int    uUnitInLen;        /* 输出 unit 长度 */
    unsigned char * pUnitInBuf;        /* 输入 unit 指针 */

	unsigned char *	pOutBuf;			/* 输出缓冲区           */
	unsigned int	uOutBufLen;	    /* 输出缓冲区长度       */
	unsigned int	uOutBufSize;	    /* 输出缓冲区大小       */

	unsigned int    uCompanyMark;       /* 公司描述符           */
	unsigned int    uCameraMark;        /* 相机描述             */

    unsigned int    uRes[1];             /* 预留，兼容64位平台*/
    
    MEDIAREFER_TIME_T stGlobalTime;        /* 全局时间                             */
}MEDIAPS_COMPLEX_PROCESS_PARAM_T;

/******************************************************************************
* 接口函数声明
******************************************************************************/

/******************************************************************************
* 功  能：获取所需内存大小
* 参  数：param - 参数结构指针
* 返回值：返回错误码
* 备  注：参数结构中 buffer_size变量用来表示所需内存大小
******************************************************************************/
int MediaPs_GetMemSize(MEDIAPS_COMPLEX_PARAM_T *param); 

/******************************************************************************
* 功  能：创建PSMUX模块
* 参  数：param  	- 参数结构指针
*         **handle	- 返回PSMUX模块句柄
* 返回值：返回错误码
******************************************************************************/
int MediaPs_Create(MEDIAPS_COMPLEX_PARAM_T *param, void **handle); 

/******************************************************************************
* 功  能：复合一段数据块
* 参  数：handle - 句柄(handle由PSMUX_Create返回)
* 返回值：返回错误码
******************************************************************************/
int MediaPs_Process(void *handle, MEDIAPS_COMPLEX_PROCESS_PARAM_T *param);

/******************************************************************************
* 功  能：重置参考数据
* 参  数：handle - 句柄(handle由PSMUX_Create返回)
*         info   - 参考数据句柄
* 返回值：返回错误码
******************************************************************************/
int MediaPs_ResetStreamInfo(void *handle, MEDIAPS_ES_INFO_T *info);


#ifdef __cplusplus
}
#endif 

#endif /* _MEDIAPS_LIB_H_ */

