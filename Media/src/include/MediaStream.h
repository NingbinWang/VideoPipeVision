#ifndef  _MEDIA_STREAM_H_
#define _MEDIA_STREAM_H_
#include "Media.h"
#include "MediaFrame.h"
#include "Logger.h"
#include "Common.h"
#include "Mutex.h"
typedef enum
{
    PACK_JPEG_IMG  = 0,       /*原始图*/
    PACK_JPEG_THM  = 1,       /*缩略图*/
    PACK_YUV_IMG   = 2,       /*YUV原图*/
    PACK_JPEG_MAX  = INT_MAXI,
}MEDIA_PACK_JPEG_TYPE_E;

typedef struct  
{
    UINT                 uNaluNum;                       /* 当前nalu个数 */
    NALU_T               stNalu[MAX_NALU_NUM];            /* nalu长度 */
    UINT                 picture_mode;                  /* 帧类型 */
    UINT                 encode_type;                   /* 编码类型 */
    UINT                 bInterlace;                    /* 是否为隔行信号 */
    UINT                 timeStamp;                     /* 时间戳 */
    UINT                 frameNum;                      /* 帧号 */
    UINT                 width;                         /* 编码宽 */
    UINT                 height;                        /* 编码高 */
    BOOL                 bWaterMark;                    /* 是否使能水印 */
    BOOL                 dropMode;                      /* 丢帧策略 */
    BOOL                 bFirstFrm;                     /* 是否为开启编码后的第一帧 */
    MEDIA_ENC_PARAM_T    encCfg;                      /* 编码参数 */
    DATE_TIME_T          now;                           /* 系统时间 */
	UINT8                encrypt_round[MAX_NALU_NUM];
    BOOL                 res[1];                        /* 预留，兼容64位平台 */
	UINT8                nalutype[MAX_NALU_NUM];
} MEDIA_PACK_VIDEO_CHN_ATTR_T;

typedef struct  
{
    PUINT8        in_buf;                        /* 码流输入地址 */
    UINT          in_length;                     /* 码流输入有效长度*/
    UINT          smpRate;                       /* 采样率 */
    UINT          audioType;                     /* 编码类型 */
    UINT          timeStamp;                     /* 时间戳 */
    DATE_TIME_T   now;                           /* 系统时间 */
}  MEDIA_PACK_AUDIO_CHN_ATTR_T;



typedef struct  
{
    PUINT8      in_buf_base;                    /* 码流输入首地址,有效内容从该地址的第sizeof(ES_STREAM_ELEMENT)处开始，前面72字节预留，不用赋值 */
    PUINT8      in_buf_data;                    /* 码流数据的实际存放地址，in_buf_data = in_buf_base + sizeof(ES_STREAM_ELEMENT) */
    UINT        in_length;                      /* 码流输入有效长度，不包含预留的sizeof(ES_STREAM_ELEMENT)个字节*/
    UINT        timeStamp;                      /* 时间戳 */
    MEDIA_PACK_JPEG_TYPE_E    type;           /* 图片类型*/
    UINT        res[1];                         /* 预留，兼容64位平台*/
    DATE_TIME_T   now;                            /* 系统时间 */
} MEDIA_PACK_JPEG_CHN_ATTR_T;

typedef struct
{
    PUINT8      in_buf_base;                    /* 码流输入首地址,有效内容从该地址的第sizeof(ES_STREAM_ELEMENT)处开始，前面72字节预留，不用赋值 */
    PUINT8      in_buf_data;                    /* 码流数据的实际存放地址，in_buf_data = in_buf_base + sizeof(ES_STREAM_ELEMENT) */
    UINT        in_length;                      /* 码流输入有效长度，不包含预留的sizeof(ES_STREAM_ELEMENT)个字节*/
    UINT        timeStamp;                      /* 时间戳 */
    DATE_TIME_T   now;                            /* 系统时间 */
} MEDIA_PACK_AUDIO_DWN_ATTR_T;


typedef struct 
{
    UINT            idx;
    UINT            streamType;       //流类型
    UINT            videoStreamType;  //视频编码类型
    UINT            audioStreamType;  //音频编码类型
    UINT            width;            //视频图像宽度，与vid传来的参数设置
    UINT            height;           //视频图像高度，与vid传来的参数设置
    void           *psHandle;         //ps打包句柄
    BOOL            bChange;
	UINT            ptsBase;
    UINT            clockInterval;    //帧间隔
    BOOL            bWaterMark;       //是否添加水印
	UINT		    videoFrameNum;
	UINT            packType;         /*0:PS 1:ES*/
	BOOL            dropMode;         /*0:丢弃 1:覆盖*/
	UINT            res[1];           /* 预留，兼容64位平台*/ 
}MEDIA_STREAM_PACK_CTRL_T;

typedef struct
{
    UINT        					uNaluNum;                       /* 当前nalu个数 */
    NALU_T      					astNalu[MAX_NALU_NUM];            /* nalu长度 */
    void*       					pOut_buf;                       /* 码流输出地址 */
    UINT        					uOut_buf_length;                /* 码流输出地址缓存总长度 */
    UINT        					uOut_length;                    /* 码流输出有效长度 */
    MEDIA_ENC_FRAME_TYPE_E          eFrameType;                  /* 帧类型 */
    UINT        					uTimeStamp;
    UINT        					uFrameNum;
    DATE_TIME_T   					stNow;      /* 系统时间 */
} MEDIA_STREAM_PACK_PARAM_T;



class MediaStream
{

    /* data */
public:
    static MediaStream* createNew(MEDIA_PARAM_T& pParam);
    MediaStream(MEDIA_PARAM_T* pParam);
    ~MediaStream();
    INT32  StreamPack(MEDIA_ENC_FRAME_T* pFrame);
    INT32  StreamFrameConvert(MEDIA_VIDEO_FRAME_T* pFrameInfo,UINT uVoChan);
    INT32  SendStreamToDecPool(UINT32 uChan,PUINT8 pStreamSrc,UINT32 uLength,INT32 iDrop);
    INT32  SendStreamToEncPool(UINT32 uChan,PUINT8 pStreamSrc,UINT32 uLength,INT32 iDrop);
    INT32  SendStreamToAudioPool(PUINT8 pStreamSrc,UINT32 uLength,INT32 iDrop);
private:
	INT32  SendStreamToRecPool(UINT32 uChan,PUINT8 pStreamSrc,UINT32 uLength, BOOL bVideo, BOOL dropMode);
	MEDIA_PARAM_T* mpParam;
};







#endif
