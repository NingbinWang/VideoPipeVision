#ifndef _MEDIA_PRIV_H_
#define _MEDIA_PRIV_H_
#include "Media.h"
#include "MediaFrame.h"
#include "Logger.h"
#include "Common.h"
#include "MediaViInner.h"
#include "MediaEncInner.h"
#include "MediaStream.h"
#include "MppEncoder.h"
#include "MppDecoder.h"
#include "RKrga.h"
#include "RKnpu.h"
#include "MediaAudioInner.h"

// Media内部参数
typedef struct
{
    BOOL                    algLoad;
	MediaStream*            pStream;
	MediaVi*                apVi[MAX_VI_CHAN_SOC+ MAX_USB_CHAN_SOC];//这里有多少路硬件路就有多少个Vi
	MediaEnc*               apEnc[MAX_ENC_CHAN_SOC];
	MediaEncCallback        AICallback;
	MediaAudio*             pAudio;
}MEDIA_INNER_PARAM_T;

typedef struct
{
  int width;
  int height;
  int width_stride;
  int height_stride;
  int format;
  char *virt_addr;
  int fd;
} IMAGE_FRAME_T;


MEDIA_INNER_PARAM_T* Media_Get_InnerParam(void);
MEDIA_FORMAT_TYPE_E MediaForccFrame(char * strFormate);
RgaSURF_FORMAT MediaRgaFmtTranslation(MEDIA_FORMAT_TYPE_E eType);
VO_CFG_PARAM_T* Media_Get_VoParam(UINT32 uChan);




#ifdef MEDIARKMPP
RKrga * Media_GetRkrga(void);
RKnpu * Media_GetRknpu(void);
MppEncoder * Media_Getmppencoder(void);
MppDecoder *Media_Getmppdecoder(void);

#endif

#endif
