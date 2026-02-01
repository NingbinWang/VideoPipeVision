//该文件主要用于填写相应的描述信息
#ifndef _MEDIAREFER_H_
#define _MEDIAREFER_H_

/* 原始流类型 */
#define STREAM_TYPE_UNDEF			0x00
#define STREAM_TYPE_VIDEO_MPEG1     0x01
#define STREAM_TYPE_VIDEO_MPEG2     0x02
#define STREAM_TYPE_VIDEO_MPEG4     0x10
#define STREAM_TYPE_VIDEO_H264      0x1b    /* 标准H264       */
#define STREAM_TYPE_VIDEO_SVAC      0x80    /* 标准SVAC       */
#define STREAM_TYPE_VIDEO_MJPG		0xb1	/* 以PS流负载MJPG */
#define STREAM_TYPE_VIDEO_H265      0x24    /* 标准H265/HEVC  */

#define STREAM_TYPE_AUDIO_MPEG1     0x03
#define STREAM_TYPE_AUDIO_MPEG2     0x04
#define STREAM_TYPE_AUDIO_AAC       0x0f
#define STREAM_TYPE_AUDIO_G711A     0x90
#define STREAM_TYPE_AUDIO_G711U     0x91
#define STREAM_TYPE_AUDIO_G722_1    0x92
#define STREAM_TYPE_AUDIO_G723_1    0x93
#define STREAM_TYPE_AUDIO_G726		0x96
#define STREAM_TYPE_AUDIO_G726_16	0x98
#define STREAM_TYPE_AUDIO_G729      0x99
#define STREAM_TYPE_AUDIO_AMR_NB    0x9a
#define STREAM_TYPE_AUDIO_AMR_WB    0x9b
#define STREAM_TYPE_AUDIO_L16       0x9c
#define STREAM_TYPE_AUDIO_L8        0x9d
#define STREAM_TYPE_AUDIO_DVI4      0x9e
#define STREAM_TYPE_AUDIO_GSM       0x9f
#define STREAM_TYPE_AUDIO_GSM_EFR   0xa0
#define STREAM_TYPE_AUDIO_LPC       0xa1
#define STREAM_TYPE_AUDIO_QCELP     0xa2
#define STREAM_TYPE_AUDIO_VDVI      0xa3
#define STAEAM_TYPE_AUDIO_OPUS      0xa5 
#define STREAM_TYPE_AUDIO_AAC_LD    0xa6
#define STREAM_TYPE_PRIVT_SECTION   0x05    /* 符合 iso 13818-1定义的私有section */
#define STREAM_TYPE_PRIVT_DATA      0x06    /* 符合 iso 13818-1定义的私有数据    */


/* 输入数据类型，对应stream_mode */
#define INCLUDE_VIDEO_STREAM            (1 << 0)    /* 包含视频流 */
#define INCLUDE_AUDIO_STREAM            (1 << 1)    /* 包含音频流 */
#define INCLUDE_PRIVT_STREAM		    (1 << 2)    /* 包含私有流 */

#define NONE_INCLUDE_WATERMARK          0           /* 不包含水印  */
#define I_INCLUDE_WATERMARK             (1 << 0)    /* I帧包含水印 */
#define P_INCLUDE_WATERMARK             (1 << 1)    /* P帧包含水印 */
#define B_INCLUDE_WATERMARK             (1 << 2)    /* B帧包含水印 */
/* 输入帧类型，对应frame_type */
#define FRAME_TYPE_UNDEF                (-1)    /* 未定义的帧类型 */
#define FRAME_TYPE_PRIVT_FRAME          5       /* 私有数据帧 */
#define FRAME_TYPE_AUDIO_FRAME          4       /* 音频数据帧 */
#define FRAME_TYPE_VIDEO_IFRAME         3       /* 视频数据 I 帧 */
#define FRAME_TYPE_VIDEO_EFRAME         2       /* 视频数据 E 帧 */
#define FRAME_TYPE_VIDEO_PFRAME         1       /* 视频数据 P 帧 */
#define FRAME_TYPE_VIDEO_BFRAME		    0       /* 视频数据 B 帧 */

#define RTP_PAYLOAD_TYPE_HIK_PRIVT		0x70
#define RTP_EXT_UNDEF					0x0000
#define RTP_BASIC_STREAM_INFO			0x0001 
#define RTP_CODEC_INFO					0x0002

#define PRIVT_EXT_UNDEF				0x0000
#define PRIVT_BASIC_STREAM_INFO		0x0001 
#define PRIVT_CODEC_INFO			0x0002 
#define PRIVT_IVS_INFO				0x0003
#define PRIVT_ITS_INFO              0x0004
#define PRIVT_IAS_INFO              0x0005
#define PRIVT_MDS_INFO				0x0006
#define PRIVT_POS_INFO				0x0007
#define PRIVT_MD_INFO				0x0008
#define HIK_PRIVT_IVT_DEBUG				0x0009

/* descriptor 设置宏 */
#define INCLUDE_BASIC_DESCRIPTOR		    (1 << 0)
#define INCLUDE_DEVICE_DESCRIPTOR		    (1 << 1)
#define INCLUDE_VIDEO_DESCRIPTOR		    (1 << 2)
#define INCLUDE_AUDIO_DESCRIPTOR		    (1 << 3)
#define INCLUDE_VIDEO_CLIP_DESCRIPTOR		(1 << 4)
#define INCLUDE_TIMING_HRD_DESCRIPTOR		(1 << 5)



#define DESCRIPTOR_DEF_VERSION		0x0100  
#define DESCRIPTOR_DEF_INTRA        0x0101     // 支持intra标志的版本号
#define DESCRIPTOR_DEF_MERGE        0x0102     // 支持合成流标志的版本号
#define DESCRIPTOR_YEAR_BASE			2000

#define TIMING_HRD_DESCRIPTOR_LEN   12
#define BASIC_DESCRIPTOR_LEN		16
#define DEVICE_DESCRIPTOR_LEN       20
#define VIDEO_DESCRIPTOR_LEN        16
#define AUDIO_DESCRIPTOR_LEN        12
#define VIDEO_CLIP_DESCRIPTOR_LEN   12
#define STREAM_DESCRIPTOR_LEN		12
#define ENCRYPT_INFO_LEN		    8

#define TIMING_HRD_DESCRIPTOR_TAG	0x2A
#define BASIC_DESCRIPTOR_TAG		0x40
#define DEVICE_DESCRIPTOR_TAG       0x41
#define VIDEO_DESCRIPTOR_TAG        0x42
#define AUDIO_DESCRIPTOR_TAG        0x43
#define VIDEO_CLIP_DESCRIPTOR_TAG   0x44
#define STREAM_DESCRIPTOR_TAG		0x45
#define STREAM_ENCRYPT_TAG		    0x80

#define PAYLOAD_TYPE_ALL_VIDEO      0x60 ///<通用视频PayloadType值
#define PAYLOAD_TYPE_MJPEG_VIDEO    0x1a ///<MJPEG视频PayloadType值
#define PAYLOAD_TYPE_MPEG2_VIDEO    0x20 ///<MPEG2视频PayloadType值
#define PAYLOAD_TYPE_MPEJA_AUDIO    0x0e ///<MPEJA音频PayloadType值
#define PAYLOAD_TYPE_AAC_AUDIO      0x68 ///<AAC音频PayloadType值


/* 公司类型定义 */
#define CO_TYPE_UNDEF                        0x00
#define CO_TYPE_PANASONIC                    0x01
#define CO_TYPE_SONY                         0x02
#define CO_TYPE_AXIS                         0x03
#define CO_TYPE_SANYO                        0x04
#define CO_TYPE_BOSCH                        0x05
#define CO_TYPE_ZAVIO                        0x06
#define CO_TYPE_GRANDEYE                     0x07
#define CO_TYPE_PROVIDEO                     0x08
#define CO_TYPE_ARECONT                      0x09
#define CO_TYPE_ACTI                         0x0a
#define CO_TYPE_PELCO                        0x0b
#define CO_TYPE_VIVOTEK                      0x0c
#define CO_TYPE_INFINOVA                     0x0d
#define CO_TYPE_ONVIF                        0x0e
#define CO_TYPE_PSIA                         0x0f
#define CO_TYPE_SAMSUNG                      0x10
#define CO_TYPE_GE105E                       0x11
#define CO_TYPE_GE105E_IPC                   0x12
#define CO_TYPE_CAMPLUS2                     0x13
#define CO_TYPE_TVCMPX                       0x14
#define CO_TYPE_DYNIPC                       0x15


/* 相机类型定义 */
#define CAMERA_TYPE_UNDEF                    0x00

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	unsigned int	stream_type;		/* 视频流类型 */
	unsigned int	frame_num;			/* 视频帧号 */

    unsigned int    encoder_version;    /* 编码器版本   */
    unsigned int    encoder_year;       /* 编码器日期 年 */
    unsigned int    encoder_month;      /* 编码器日期 月 */
    unsigned int    encoder_date;       /* 编码器日期 日 */

    unsigned int    width_orig;			/* 编码图像宽高，16的倍数*/
    unsigned int    height_orig;      
    unsigned int    interlace;			/* 是否场编码   */
    unsigned int    b_frame_num;		/* b帧个数      */
	unsigned int    is_svc_stream;      /* svc码流标识，HIK_basic_descriptor中def_Version从版本0x0100开始，0表示为svc码流，1不是svc码流*/
    unsigned int    use_e_frame;		/* 是否使用e帧  */
    unsigned int    max_ref_num;		/* 最大参考帧个数 */
    unsigned int    fixed_frame_rate;	/* 是否固定帧率 */
    unsigned int    time_info;			/* 以1/90000 秒为单位的两帧间时间间隔 */
    unsigned int    watermark_type;		/* 水印类型 */
	unsigned int    deinterlace;        /* 显示时是否需要反隔行 */
	unsigned int    min_search_blk;     /* 最小搜索块大小*/
	unsigned int    light_storage;      /* 是否轻存储*/
    unsigned int    play_clip;			/* 是否需要显示裁剪 */
    unsigned int    start_pos_x;		/* 裁剪后显示区域左上角点在原区域的坐标x */
    unsigned int    start_pos_y;		/* 裁剪后显示区域左上角点在原区域的坐标y */
    unsigned int    width_play;			/* 裁剪后显示区域的宽度 */
    unsigned int    height_play;		/* 裁剪后显示区域的高度 */
    unsigned int    intra_refresh;      /* intra标志，01表示为intra码流，10表示为Infinit  GOP码流 */
}MEDIAREFER_VIDEO_INFO_T;

typedef struct
{
	unsigned int	stream_type;		/* 音频流类型		*/
    unsigned int    frame_len;			/* 音频帧长度 */
    unsigned int    audio_num;			/* 音频声道数 0 单声道，1 双声道*/
    unsigned int    sample_rate;		/* 音频采样率 */
    unsigned int    bit_rate;			/* 音频码率 */
    unsigned int    bits_per_sample;   
} MEDIAREFER_AUDIO_INFO_T;

typedef struct
{
    unsigned int    year;        /* 全局时间年，2008 年输入2008*/
    unsigned int    month;       /* 全局时间月，1 - 12*/
    unsigned int    date;        /* 全局时间日，1 - 31*/
    unsigned int    hour;        /* 全局时间时，0 - 23*/
    unsigned int    minute;      /* 全局时间分，0 - 59*/
    unsigned int    second;      /* 全局时间秒，0 - 59*/
    unsigned int    msecond;     /* 全局时间毫秒，0 - 999*/
} MEDIAREFER_TIME_T;

typedef struct
{
    unsigned int    is_hik_stream;         /* 是否是符合海康定义的描述字   */
	unsigned int	privt_stream_type;	   /* 私有流类型					*/
    unsigned int    encrypt_type;          /* 加密类型                     */
	unsigned int    dev_chan_id_flg;       /* 设备通道号信息是否可用，1为是，0为否 */
    unsigned char   dev_chan_id[16];       /* 设备和通道 id                */  
    MEDIAREFER_VIDEO_INFO_T  video_info;            /* 视频信息                     */  
    MEDIAREFER_AUDIO_INFO_T  audio_info;            /* 音频信息                     */
    MEDIAREFER_TIME_T glb_time;              /* 全局时间                     */
	unsigned int    is_hik_mergstream; /*是否为合成流*/
} MEDIAREFER_STREAM_INFO;


unsigned int MediaRefer_fill_basic_descriptor(unsigned char *buffer,  MEDIAREFER_TIME_T *glb_time,  unsigned int encrypt_type, unsigned int company_mark,unsigned int camera_mark);
unsigned int MediaRefer_fill_stream_descriptor(unsigned char *buffer, unsigned int video_stream_type, unsigned int audio_stream_type,unsigned int video_frame_num);
unsigned int MediaRefer_fill_video_descriptor(unsigned char *buffer, MEDIAREFER_VIDEO_INFO_T *video_info);
unsigned int MediaRefer_fill_audio_descriptor(unsigned char *buffer, MEDIAREFER_AUDIO_INFO_T *audio_info);
unsigned int MediaRefer_fill_device_descriptor(unsigned char *buffer, unsigned char dev_chan_id[16]);
unsigned int MediaRefer_fill_video_clip_descriptor(unsigned char *buffer, MEDIAREFER_VIDEO_INFO_T *video_info);
unsigned int MediaRefer_fill_timing_hrd_descriptor(unsigned char *buffer, int frame_rate, int width, int height);

#ifdef __cplusplus
}
#endif 


#endif

