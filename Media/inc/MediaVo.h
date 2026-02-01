#ifndef _MEDIA_VO_H_
#define _MEDIA_VO_H_
#include "Common.h"
#include "MediaConfig.h"
#include "MediaFrame.h"
#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/



/*****************************LOGO参数**********************************/
/*LOGO配置参数*/
#define LOGO_MAX_W		(512)     /*宽度*/
#define LOGO_MAX_H		(256)     /*高度*/
#define OSD_LOGO_LEN		(LOGO_MAX_W * LOGO_MAX_H * 2)

typedef struct
{
	UINT                              uChan; //Vo的通道
	UINT                              uDecChan;//使用解码pool的通道
	UINT32                            u32ImageWidth;//Vo的输出大小
	UINT32                            u32ImageHeight;
	UINT32                            u32FrameSize;
	MEDIA_FORMAT_TYPE_E				  eType;
    UINT32                            res[62];
    /**< 预留*/
}VO_CFG_PARAM_T;






#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif