#include "MediaPriv.h"
#include "New.h"
#include "SysMemory.h"


MEDIA_PARAM_T* pMediaInitParam = NULL;
MEDIA_INNER_PARAM_T* pInnerParam = NULL;



VI_CFG_PARAM_T* Media_Get_ViParam(UINT32 uChan)
{
	if(pMediaInitParam == NULL){
		printf("[%s:%d]Media_Get_ViParam error YOU MSUT init media param\n", __FUNCTION__, __LINE__);
		return NULL;
	}
    return &pMediaInitParam->astViCfgParam[uChan];
}

VO_CFG_PARAM_T* Media_Get_VoParam(UINT32 uChan)
{
	if(pMediaInitParam == NULL){
		printf("[%s:%d]Media_Get_ViParam error YOU MSUT init media param\n", __FUNCTION__, __LINE__);
		return NULL;
	}
    return &pMediaInitParam->astVoCfgParam[uChan];
}



MEDIA_INNER_PARAM_T* Media_Get_InnerParam(void)
{
	return pInnerParam;
}


#ifdef MEDIARKMPP
RKrga *rkrga = nullptr;
RKnpu * rknpu = nullptr;
MppEncoder * mppencoder = nullptr;
MppDecoder * mppdecoder = nullptr;
RKrga * Media_GetRkrga(void)
{
	return rkrga;
}
RKnpu * Media_GetRknpu(void)
{
	return rknpu;
}
MppEncoder * Media_Getmppencoder(void)
{
	return mppencoder;
}

MppDecoder *Media_Getmppdecoder(void)
{
	return mppdecoder;
}



RgaSURF_FORMAT MediaRgaFmtTranslation(MEDIA_FORMAT_TYPE_E eType)
{
    switch(eType){
			case MEDIA_FORMAT_YUV420SP:
				return RK_FORMAT_YCbCr_420_SP;
			case MEDIA_FORMAT_BGRA8888:
				return RK_FORMAT_BGRA_8888;
			default:
				return RK_FORMAT_UNKNOWN;
    }
	return RK_FORMAT_UNKNOWN;
}






#endif

MEDIA_FORMAT_TYPE_E MediaForccFrame(char * strFormate)
{
   if(!strcmp(strFormate,"NV12")){
		return MEDIA_FORMAT_YUV420SP;
	}
	return MEDIA_FORMAT_MAX;
	
}

INT32 MediaInit(MEDIA_PARAM_T* pParam)
{
	UINT32 u32Chan = 0;
	if(pParam == NULL)
	{
		return ERROR;
	}
	pMediaInitParam = pParam;
#ifdef MEDIARKMPP
		rkrga = new RKrga();
		rknpu = new RKnpu();
		mppencoder = new MppEncoder();
		mppdecoder = new MppDecoder();
#endif
	pInnerParam = (MEDIA_INNER_PARAM_T*)SysMemory_malloc(sizeof(MEDIA_INNER_PARAM_T));
	if(!pInnerParam){
		LOG_ERROR("Malloc MEDIA_INNER_PARAM_T fail \n");
	}
	for(u32Chan = 0;u32Chan < pParam->u32ViChanCnt;u32Chan++){
		pParam->astViCfgParam[u32Chan].u32Chn = u32Chan;
		pInnerParam->apVi[u32Chan] = MediaVi::createNew(pParam->astViCfgParam[u32Chan]);
	}
	for(u32Chan = 0;u32Chan < pParam->u32EncChanCnt;u32Chan++){
		pParam->astEncCfgParam[u32Chan].uChan = u32Chan;
		pInnerParam->apEnc[u32Chan] = MediaEnc::createNew(pParam->astEncCfgParam[u32Chan]);
	}
	//stream
	pInnerParam->pStream = MediaStream::createNew(*pParam);
	//audio
	pInnerParam->pAudio = MediaAudio::createNew(pParam->stAudioCfgParam);

	pInnerParam->apEnc[0]->MediaEncStartThread();

	
    return 0;
}


