#ifndef _MEDIADECINNER_H_
#define _MEDIADECINNER_H_
#include "Media.h"
#include "Logger.h"
#include "Common.h"
#include "MediaPriv.h"



int MediaDecReSize(IMAGE_FRAME_T* srcimg,IMAGE_FRAME_T *outimg);

bool MediaDecCvtcoloerVirt(IMAGE_FRAME_T* srcimg,IMAGE_FRAME_T *outimg);




#endif

