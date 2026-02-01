#ifndef _MEDIAMANAGER_H_
#define _MEDIAMANAGER_H_
#include "Media.h"
int MediaManagerInit();
MEDIA_PARAM_T * MediaManagerGet();
unsigned int  GetDecStream(unsigned int uChan,void *pUserData,INT32 iDrop);
unsigned int  GetEncStream(unsigned int uChan,void *pUserData);

#endif