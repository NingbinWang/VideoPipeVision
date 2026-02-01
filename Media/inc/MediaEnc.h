#ifndef _MEDIA_ENC_H_
#define _MEDIA_ENC_H_
#include "MediaConfig.h"
#include "MediaFrame.h"
#include "Common.h"
#include "SysMutex.h"

#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

typedef enum {
    MEDIA_ENC_CODETYPE_Unused,             /**< Value when coding is N/A */
    MEDIA_ENC_CODETYPE_AutoDetect,         /**< Autodetection of coding type */
    MEDIA_ENC_CODETYPE_MPEG2,              /**< AKA: H.262 */
    MEDIA_ENC_CODETYPE_H263,               /**< H.263 */
    MEDIA_ENC_CODETYPE_MPEG4,              /**< MPEG-4 */
    MEDIA_ENC_CODETYPE_WMV,                /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    MEDIA_ENC_CODETYPE_RV,                 /**< all versions of Real Video */
    MEDIA_ENC_CODETYPE_AVC,                /**< H.264/AVC */
    MEDIA_ENC_CODETYPE_MJPEG,              /**< Motion JPEG */
    MEDIA_ENC_CODETYPE_VP8,                /**< VP8 */
    MEDIA_ENC_CODETYPE_VP9,                /**< VP9 */
    MEDIA_ENC_CODETYPE_VC1 = 0x01000000,   /**< Windows Media Video (WMV1,WMV2,WMV3)*/
    MEDIA_ENC_CODETYPE_FLV1,               /**< Sorenson H.263 */
    MEDIA_ENC_CODETYPE_DIVX3,              /**< DIVX3 */
    MEDIA_ENC_CODETYPE_VP6,
    MEDIA_ENC_CODETYPE_HEVC,               /**< H.265/HEVC */
    MEDIA_ENC_CODETYPE_AVSPLUS,            /**< AVS+ */
    MEDIA_ENC_CODETYPE_AVS,                /**< AVS profile=0x20 */
    MEDIA_ENC_CODETYPE_AVS2,               /**< AVS2 */
    MEDIA_ENC_CODETYPE_AV1,                /**< av1 */
    MEDIA_ENC_CODETYPE_KhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
    MEDIA_ENC_CODETYPE_VendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
    MEDIA_ENC_CODETYPE_Max = 0x7FFFFFFF
}  MEDIA_ENC_CODETYPE_E;

/* Rate control parameter */
typedef enum {
    MEDIA_ENC_RCMODE_VBR,
    MEDIA_ENC_RCMODE_CBR,
    MEDIA_ENC_RCMODE_FIXQP,
    MEDIA_ENC_RCMODE_AVBR,
    MEDIA_ENC_RCMODE_SMTRC,
    MEDIA_ENC_RCMODE_BUTT
} MEDIA_ENC_RCMODE_E;

typedef enum
{
    ENC_FRM_I   = 0,
    ENC_FRM_P   = 1,
    ENC_FRM_B   = 2,
    ENC_FRM_MAX,
}MEDIA_ENC_FRAME_TYPE_E;


typedef struct 
{
    BOOL       bEnable;                               /*是否打开*/
   	UINT32	   encW;			 /*编码宽高*/
	UINT32	   encH;			 /*编码宽高*/
	UINT32	   viLostFrm; 	 /*采集丢帧数目累加*/
    UINT       uEncfps;                               /*实时帧率*/
    UINT       uCfgEncbps;                            /*配置的码率*/
    UINT       uCfgbpsType;                           /*码率控制类型 0: CBR, 1: VBR */
    UINT       uStatTm;                               /*码率统计时间*/
    UINT       uEncbps;                               /*实际统计码率*/
    UINT       uIFrameInterval;                       /*配置的I帧间隔*/
    UINT       uIDRInterval;                          /*配置的IDR帧间隔*/
	UINT32      encBitRate;	  /*编码比特率*/
}MEDIA_ENCODER_BASIC_PARAM_T;

/*ENC 高级参数*/ 
typedef struct
{
    UINT    uProfile;                              /*编码等级*/
    UINT    uMaxPqp;                               /*p帧最大qp*/
    UINT    uMinPqp;                               /*p帧最小qp*/
    UINT    uMaxIqp;                               /*i帧最大qp*/
    UINT    uMinIqp;                               /*i帧最小qp*/
}MEDIA_ENCODER_ADV_PARAM_T;

/*ENC 统计参数*/ 
typedef struct
{
    UINT    uEncFrmCnt;                            /*已编码帧数统计*/
    UINT    uIFrameNum;                            /*已编码I帧数统计*/                 
    UINT    uPFrameNum;                            /*已编码P帧数统计*/
    UINT    uEncLostFrm;                           /*编码丢帧数目累加*/
    UINT    uEncResetCnt;                          /*编码器重新配置次数*/
    UINT    uEncErrCnt;                            /*编码器配置出错次数*/
}MEDIA_ENCODER_STAT_PARAM_T;

//编码器的状态信息
typedef struct
{   
    MEDIA_ENCODER_BASIC_PARAM_T  stEncBasicParam;       /*编码基础参数*/
    MEDIA_ENCODER_ADV_PARAM_T    stEncAdvParam;         /*编码高级参数*/
    MEDIA_ENCODER_STAT_PARAM_T   stEncStatParam;        /*编码统计参数*/
}MEDIA_ENCODER_STATUS_T;

/*编码后码流共享缓存接口定义*/
typedef struct 
{
	MUTEX_ID                 mEncPool;
    PUINT8               	 addr[MAX_SHARE_ADDR];   /*多核/多进程访问地址 */
    UINT32                   totalLen;       		/*编码缓冲长度*/
    UINT32                   wIdx;           		/*编码缓冲写索引*/
    UINT32                   rIdx;           		/*编码缓冲读索引*/
	UINT8		             res[4];
}ENC_SHARE_BUF_T;



typedef struct 
{
   BOOL                   bStart;          /*通道使能*/
   MEDIA_ENC_CODETYPE_E   uEncType;        /*编码格式*/
   UINT                   uEncW;           /*编码宽*/
   UINT                   uEncH;           /*编码高*/
   UINT   uEncFps;         /*编码帧率*/
   UINT   uEncBps;         /*编码码率*/
   UINT   uEncBpsType;     /*编码码率控制方式*/
   UINT   uEncGop;         /*编码I帧间隔*/
   UINT   uEncFrm;         /*编码帧数*/
   UINT   uEncLostFrm;     /*编码丢帧数目累加*/
   UINT   uaEncType;        /*音频编码格式*/
   UINT   uaEncFrm;         /*音频编码帧数*/
   UINT   uaEncLostFrm;     /*音频编码丢帧数目累加*/
   UINT   uaEncDenoise;     /*音频降噪*/
   UINT   packType;        /*封装流类型*/
   UINT   dropMode;        /*覆盖方式*/
   UINT   stremType;       /*媒体流类型*/
}MEDIA_ENC_STATUS_T;


typedef struct
{
	UINT                            uChan;                 /*编码通道*/
    MEDIA_ENC_CODETYPE_E            eEncoderType;          /* 编码类型*/
    CHAR                            strStreamType[6];      // 输入视频流的类型
    UINT                            uEncW;                 /* 编码输入宽 */
    UINT                            uEncH;                 /* 编码输入高 */
    MEDIA_ENC_RCMODE_E              eRcMode;               /* 码率控制类型(0:变码率;1:定码率;2:AVBR) */
    UINT                            uCfgEncfps;            /* 编码帧率 */
	MEDIA_ENC_STATUS_T              stEncStatus;           //编码的状态
} MEDIA_ENC_PARAM_T;


typedef struct
{
	UINT                        uChan;                 /*编码通道*/
	MEDIA_VIDEO_FRAME_T 		stVideoFrame;
	UINT64						u64TimeStamp;		   /* 时间戳 */
	UINT32						u32NaluNum; 		   /* nalu个数 */
	UINT32						res[1]; 				/* 预留，兼容64位平台*/
	NALU_T						astNalu[MAX_NALU_NUM];	/* nalu长度 */
}MEDIA_ENC_FRAME_T;


#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif
