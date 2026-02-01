#ifndef _MEDIAAIINNER_H_
#define _MEDIAAIINNER_H_
#include "Media.h"
#include "Logger.h"
#include "Common.h"
#include "MediaPriv.h"


int MediaAi_VideoReport(IMAGE_FRAME_T* img,DETECT_RESULT_GROUP_T *results);
int MediaAi_VideoDrawRect(IMAGE_FRAME_T* img,DETECT_RESULT_GROUP_T *detect_result);


#endif


