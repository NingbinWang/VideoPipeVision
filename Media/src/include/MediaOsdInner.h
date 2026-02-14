#ifndef _MEDIAOSDINNER_H_
#define _MEDIAOSDINNER_H_
#include "Media.h"
#include "MediaFrame.h"
#include "Logger.h"
#include "Common.h"
#include <thread>
#include "MppEncoder.h"
#include "SysMutex.h"

// 前置声明cv::Mat类型
namespace cv {
    class Mat;
}

class MediaOsd
{
	public:
		static MediaOsd* createNew(MEDIA_PARAM_T&Params);
		MediaOsd(MEDIA_PARAM_T* pParams);
		~MediaOsd();
		INT32 MediaOsdSetTime(MEDIA_VIDEO_FRAME_T *pFrame);
		INT32 MediaOsdOpenCvSetTime(MEDIA_VIDEO_FRAME_T *pFrame);
		
	private:
		void initializeOsdData(OSD_T* osdData, IMAGE_T* srcimg);
		void createTimeString(char* timeStr, int bufSize);
		void drawTimestampOnImage(cv::Mat& img, const char* timestamp, int x, int y);
};


#endif