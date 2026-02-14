#ifndef _MEDIA_VI_H_
#define _MEDIA_VI_H_
#include "Common.h"
#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#define VI_DEVNAME_STR_LEN   64

//视频输入配置参数
typedef enum
{
    VIEW_NATURAL = 0,              //不镜像
    VIEW_90FLIP,                   //90度旋转
    VIEW_180FLIP,                  //180度旋转或中心镜像
    VIEW_270FLIP,                  //270度旋转
    VIEW_VERTICAL_MIRROR,          //垂直镜像，即图像左右翻转
    VIEW_HORIZONTAL_MIRROR,        //水平镜像，即图像上下翻转
    VIEW_90FLIP_VERTICAL_MIRROR,   //90度旋转加垂直镜像
    VIEW_90FLIP_HORIZONTAL_MIRROR, //90度旋转加水平镜像
    VIEW_FLIP_MAX,
} VI_VIEW_FLIP_E;

typedef enum
{
    CMOS_OV_5969 = 0,
    CMOS_OV_8858 = 1,
	VI_SENSOR_TYPE_MAX,
}VI_SENSOR_TYPE_E;

typedef enum
{
    VI_V4L2 = 0,
    VI_HDAL = 1,
	VI_TYPE_MAX,
}VI_INPUT_TYPE_E;


typedef struct
{
	UINT32   u32DayNightMode; //白天黑夜模式切换 0:白天 1:夜晚 2：自动
	UINT8    u8Sensitive; //灵敏度0-7
} VI_DATNIGHT_INFO_T;

typedef struct
{
	UINT8    u8NRMode; //降噪开关 0:关闭 1:普通 2:专家
	UINT8    u8NRLeve; //降噪等级（普通降噪等级）0-100
	unsigned char SpaNRLeve; //降噪等级（空域降噪等级）0-100
	unsigned char TemNRLeve; //降噪等级（时域降噪等级）0-100
} VI_NR_INFO_T;

typedef struct
{
	unsigned char uWhiteBalanceMode; //白平衡模式 0:手动 1:自动白平衡1 3:锁定白平衡 6:日光灯14：白炽灯15：暖光灯16：自然光
	unsigned char R_GAIN; //白平衡等级（手动模式生效）0-100
	unsigned char B_GAIN;
} VI_WHITEBALANCE_INFO_T;

typedef struct
{
	unsigned char FogMode; //透雾模式
    unsigned char FogLeavel; //透雾等级
} VI_FOG_INFO_T;

typedef struct
{
    unsigned int x; //图像中心水平坐标
    unsigned int y; //图像中心垂直坐标
    unsigned int w; //图像窗口宽最大不能超过VIDEO_PARAM_CFG结构体中image_viW, 最小64
    unsigned int h; //图像窗口高最大不能超过VIDEO_PARAM_CFG结构体中image_viH, 最小32
} VI_RECT_T;

typedef struct
{
	unsigned int idx;//区域id
    unsigned int x; //图像中心水平坐标
    unsigned int y; //图像中心垂直坐标
    unsigned int w; //图像窗口宽最大不能超过VIDEO_PARAM_CFG结构体中image_viW, 最小64
    unsigned int h; //图像窗口高最大不能超过VIDEO_PARAM_CFG结构体中image_viH, 最小32
} VI_COVER_T;

typedef struct
{
	unsigned char BLCMode;//背光补偿模式 0:关闭 1:上 2:下 3：左，4：右5：中6：自定义
    VI_RECT_T	blc_window_rec; //补偿区域（自定义模式生效）
} VI_BLC_INFO_T;

typedef struct
{
	// 遮挡块数 
	unsigned char CoverNum;
	// 遮挡区域（自定义模式生效） 
    VI_COVER_T	cover_window_rec[4];
} VI_COVER_INFO_T;

typedef struct
{
    BOOL                                   u32Enable;            //是否进行对该接入进行使能
    UINT32                                 u32Chn;
    VI_INPUT_TYPE_E                        eViType;              //Vi的输入框架
    UINT32                                 u32Image_viW;         //前端原始图像有效宽
    UINT32                                 u32Image_viH;         //前端原始图像有效高
    UINT32                                 u32Frame_rate;        //前端帧率
    VI_SENSOR_TYPE_E                       eSensorType;          //Sensor类型
    CHAR                                   strDevname[VI_DEVNAME_STR_LEN];    //设备节点名
    CHAR                                   strFormat[6];
    VI_VIEW_FLIP_E                         eViewMirror;          // 镜像选择方式 默认为0 这个是否在编码的时候才能用到
    VI_DATNIGHT_INFO_T                     stDayNightInfo;       //日夜模式   	0-day 1-night 2-自动模式 默认为2
    UINT8                                  u8EnWdr;              //宽动态开关 0:关闭 1:打开2：自动
    UINT8                                  u8WdrLevel;           //宽动态等级（只有在宽动态开启时生效）默认为50
    unsigned char                         uBrightnessLevel;  //亮度0-100> 默认为50
    unsigned char                         uStaturationLevel; // 饱和度0-100 默认为50
    unsigned char                         uContrastLevel;    //对比度0-100 默认为50
    unsigned char                         bEnableDCE;        //畸变校正 默认为0
    unsigned char                         uSharpnessLevel;   //锐度0-100 默认为50
    unsigned int                          uExposure;         //曝光时间 [10,1000000] 微妙，默认为1/帧率
    unsigned char                          uIrisType;        //光圈类型0-DC 1-P     暂不支持此接口
    VI_WHITEBALANCE_INFO_T                uWhiteBalanceInfo; //白平衡模式 0-手动白平衡；1-自动白平衡1；3-锁定白平衡；6-日光灯；14-白炽灯；15-暖光灯；16-自然光 默认为1
    VI_NR_INFO_T                          uNRInfo;           //数字降噪	  0-关闭  1-普通 2-专家 默认为1
    VI_FOG_INFO_T                         uFogInfo;          //透雾模式 0 关 1 开启 2自动
    unsigned char                         uElesharkMode;     //电子防抖开关
    VI_BLC_INFO_T                         uBLCInfo;          //背光补偿 默认关
    unsigned char                         uHLCMode;          //强光抑制 默认关
    unsigned char                         enHLCLevel;        //强光抑制等级（只有在强光抑制开启时生效）默认为50    
    VI_COVER_INFO_T                       uCoverMode;        //视频遮挡 默认关   
	unsigned char                         ulightover;        //防补光过爆0关 1开 默认为0
	unsigned char                         ugrayscale;        //灰度范围0[0-255] 1[16-235]默认为0
    unsigned char                         res[7];
} VI_CFG_PARAM_T;

#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif