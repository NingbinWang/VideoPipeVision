#include "Media.h"
#include "MppEncoder.h"
#include "Logger.h"
#include <time.h>
#include <sys/time.h>
#include "MediaPriv.h"

INT32 MediaOsdInit(VOID)
{
    return 0;
}


INT32 MediaOsdSetTime(IMAGE_FRAME_T* srcimg,IMAGE_FRAME_T *outimg)
{
    return 0;
}


// 获取当前时间戳（格式：YYYY-MM-DD HH:MM:SS）
//void get_current_time(char *time_str, int buffer_size) {
//    time_t raw_time;
//    struct tm *time_info;
//    struct timeval tv;
//    char milliseconds[4];

    // 获取秒级时间 + 毫秒
//    gettimeofday(&tv, NULL);
 //   time(&raw_time);
//    time_info = localtime(&raw_time);

    // 格式化为字符串
//    strftime(time_str, buffer_size, "%Y-%m-%d %H:%M:%S", time_info);

    // 添加毫秒（可选）
//    snprintf(milliseconds, sizeof(milliseconds), ".%03d", (int)(tv.tv_usec / 1000));
//    strncat(time_str, milliseconds, buffer_size - strlen(time_str) - 1);
//}