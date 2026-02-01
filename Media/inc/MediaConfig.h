#ifndef _MEDIA_CONFIG_H_
#define _MEDIA_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif/*__cplusplus*/

#ifdef HDAL_DEF
#define MAX_MULTI_STREAM_CHAN       (2)
#define MAX_SHARE_ADDR       	   (4)  
#define OSD_MAX_LINE                    (3)
#define BEEP_MAX_SOUND_NUM              (64)

#define MEDIA_SHARE_SEGMENT_ID	(100)

#define SHMSHARE 0

#define MEDIA_VI_MAX_DEV_NUM	4
#define MEDIA_VI_MAX_CHN_NUM	4
#define MEDIA_VI_MAX_PATH_NUM	8

#define MEDIA_VI_MAX_CHIP_CNT			2           /*芯片内部VI 硬件chip个数*/
#define MEDIA_VI_MAX_VCAP_PER_CHIP	1           /*每个VI硬件chip内部包含的VCAP个数*/
#define MEDIA_VI_MAX_VI_PER_VCAP		4           /*每个VCAP内部包含的VI设备个数*/
#define MEDIA_VI_MAX_CHN_PER_VI		4           /*每个VI设备内部包含的通道个数*/
/*解码通道个数*/
#define MAX_DEC_CHAN_SOC  0
/*显示通道个数*/
#define MAX_VO_CHAN_SOC   0
/*USB摄像头通道个数*/
#define MAX_USB_CHAN_SOC  0
#define MAX_VI_CHAN_SOC 8
/*编码通道个数*/
#define MAX_ENC_CHAN_SOC      16


#endif

#ifdef MEDIARKMPP
#define MAX_MULTI_STREAM_CHAN       (2)
#define MAX_SHARE_ADDR       	   (2)  
#define OSD_MAX_LINE                    (3)
#define BEEP_MAX_SOUND_NUM              (64)

#define MEDIA_SHARE_SEGMENT_ID	(100)

#define SHMSHARE 0

#define MEDIA_VI_MAX_DEV_NUM	4
#define MEDIA_VI_MAX_CHN_NUM	4
#define MEDIA_VI_MAX_PATH_NUM	8
/*芯片内部VI 硬件chip个数*/
#define MEDIA_VI_MAX_CHIP_CNT			2           
/*每个VI硬件chip内部包含的VCAP个数*/
#define MEDIA_VI_MAX_VCAP_PER_CHIP	1           
/*每个VCAP内部包含的VI设备个数*/
#define MEDIA_VI_MAX_VI_PER_VCAP		4           
/*每个VI设备内部包含的通道个数*/
#define MEDIA_VI_MAX_CHN_PER_VI		4           
/*解码通道个数*/
#define MAX_DEC_CHAN_SOC  0
/*显示通道个数*/
#define MAX_VO_CHAN_SOC   1
/*USB摄像头通道个数*/
#define MAX_USB_CHAN_SOC  0

#define MAX_VI_CHAN_SOC   1
/*编码通道个数*/
#define MAX_ENC_CHAN_SOC    2


#endif





#ifdef __cplusplus
}
#endif/*__cplusplus*/


#endif
