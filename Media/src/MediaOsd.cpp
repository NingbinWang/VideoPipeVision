#include "Media.h"
#include "MppEncoder.h"
#include "Logger.h"
#include "SysTime.h"
#include "MediaPriv.h"
#include "MediaOsdInner.h"
#include "New.h"
#include "RKrga.h"
#include <cstring>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include "SysMemory.h"

MediaOsd* MediaOsd::createNew(MEDIA_PARAM_T&pParams)
{
    return New<MediaOsd>::allocate(&pParams);
}

MediaOsd::MediaOsd(MEDIA_PARAM_T *pParams)
{
    // 构造函数可以根据需要添加初始化代码
}

MediaOsd::~MediaOsd() 
{
    // 析构函数可以根据需要添加清理代码
}

void MediaOsd::createTimeString(char* timeStr, int bufSize)
{
    if (!timeStr || bufSize <= 0) {
        LOG_ERROR("Invalid input parameters for time string creation\n");
        return;
    }

    SYS_DATE_TIME_T sysTime;
    INT32 ret = SysTime_get_in_struct(&sysTime);
    if (ret != OK) {
        LOG_ERROR("Failed to get system time\n");
        snprintf(timeStr, bufSize, "0000-00-00 00:00:00");
        return;
    }

    // 格式化时间字符串 YYYY-MM-DD HH:MM:SS
    snprintf(timeStr, bufSize, "%04d-%02d-%02d %02d:%02d:%02d",
            sysTime.uYear, sysTime.uMonth, sysTime.uDay,
            sysTime.uHour, sysTime.uMinute, sysTime.uSec);
}

void MediaOsd::drawTimestampOnImage(cv::Mat& img, const char* timestamp, int x, int y)
{
    if (!timestamp) {
        LOG_ERROR("Invalid timestamp string\n");
        return;
    }

    // 设置文本属性
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 0.6;
    cv::Scalar color(0, 255, 0); // 绿色
    int thickness = 2;
    int baseline = 0;

    // 计算文本尺寸，以便调整位置
    cv::Size textSize = cv::getTextSize(timestamp, fontFace, fontScale, thickness, &baseline);
    baseline += thickness;

    // 确保文本不会超出图像边界
    if (x + textSize.width > img.cols) {
        x = std::max(0, img.cols - textSize.width);
    }
    if (y + textSize.height > img.rows) {
        y = std::max(textSize.height, img.rows);
    }

    // 在图像上绘制文本
    cv::Point textOrg(x, y);
    cv::putText(img, timestamp, textOrg, fontFace, fontScale, color, thickness);
}

void MediaOsd::initializeOsdData(OSD_T* osdData, IMAGE_T* srcimg)
{
    if (!osdData || !srcimg) {
        LOG_ERROR("Invalid input parameters\n");
        return;
    }

    // 清零结构体
    memset(osdData, 0, sizeof(OSD_T));
    
    // 创建时间字符串
    char timeStr[32]; // 格式为 "YYYY-MM-DD HH:MM:SS"
    createTimeString(timeStr, sizeof(timeStr));
    
    // 计算OSD图像参数
    osdData->osdimg.width = 200; // OSD图像宽度，足够容纳时间字符串
    osdData->osdimg.height = 40; // OSD图像高度
    
    // 设置OSD图像格式，与源图像保持一致
    osdData->osdimg.format = srcimg->format; // 使用相同格式
    
    // 对于NV12格式，stride通常是宽度对齐到某边界（如16或32字节）
    osdData->osdimg.width_stride = (osdData->osdimg.width + 15) & (~15); // 对齐到16字节边界
    osdData->osdimg.height_stride = osdData->osdimg.height;
    
    // 根据图像格式分配合适的内存大小
    size_t osd_img_size;
    if (srcimg->format == MEDIA_FORMAT_YUV420SP) { // NV12格式
        // NV12: 一个Y平面 + 一个UV平面，总大小 = width_stride * height * 1.5
        osd_img_size = osdData->osdimg.width_stride * osdData->osdimg.height * 3 / 2;
    } else {
        // 默认按RGBA格式处理
        osd_img_size = osdData->osdimg.width * osdData->osdimg.height * 4;
    }
    
    osdData->osdimg.virt_addr = (CHAR *)malloc(osd_img_size);
    
    if (osdData->osdimg.virt_addr) {
        if (srcimg->format == MEDIA_FORMAT_YUV420SP) { // NV12格式
            // 对于NV12格式，我们需要创建对应的YUV数据
            // 初始化为黑色背景 (Y=0, U=V=128)
            memset(osdData->osdimg.virt_addr, 0, osdData->osdimg.width_stride * osdData->osdimg.height);
            memset(osdData->osdimg.virt_addr + osdData->osdimg.width_stride * osdData->osdimg.height, 
                   128, osdData->osdimg.width_stride * osdData->osdimg.height / 2);
            
            // 创建一个临时的灰度图像来绘制文字，然后将其转换为YUV
            cv::Mat tempMat(osdData->osdimg.height, osdData->osdimg.width, CV_8UC1);
            tempMat = cv::Scalar(0); // 黑色背景
            
            // 绘制白色的文字 (Y=255)
            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.6;
            cv::Scalar color(255); // 白色
            int thickness = 2;
            int baseline = 0;
            
            cv::Size textSize = cv::getTextSize(timeStr, fontFace, fontScale, thickness, &baseline);
            baseline += thickness;
            
            // 确保文本不会超出图像边界
            int x = std::max(5, (int)(osdData->osdimg.width - textSize.width) - 5);
            int y = std::min(osdData->osdimg.height - 5, textSize.height + baseline);
            
            cv::Point textOrg(x, y);
            cv::putText(tempMat, timeStr, textOrg, fontFace, fontScale, color, thickness);
            
            // 将灰度文字复制到Y平面上，注意使用stride
            for(int i = 0; i < osdData->osdimg.height; i++) {
                memcpy(osdData->osdimg.virt_addr + i * osdData->osdimg.width_stride,
                        tempMat.ptr(i), 
                        std::min(osdData->osdimg.width, tempMat.cols));
            }
        } else {
            // 其他格式使用RGBA处理
            cv::Mat osdMat(osdData->osdimg.height, osdData->osdimg.width, CV_8UC4, osdData->osdimg.virt_addr);
            
            // 将整个图像设置为透明黑色背景
            osdMat = cv::Scalar(0, 0, 0, 0);
            
            // 绘制时间戳文本
            drawTimestampOnImage(osdMat, timeStr, 5, osdMat.rows - 5);
        }
        
        // 设置OSD位置
        osdData->x_pos = 10; // 距离左边10像素
        osdData->y_pos = 10; // 距离顶部10像素
        
        // 设置OSD块参数 - 使用正确的字段名
        osdData->block_width = osdData->osdimg.width; // 块宽度
        osdData->block_count = 1;  // 块数量
    } else {
        LOG_ERROR("Failed to allocate memory for OSD image\n");
    }
}

INT32 MediaOsd::MediaOsdSetTime(MEDIA_VIDEO_FRAME_T *pFrame)
{
	IMAGE_T stImg;
    OSD_T stOsd;
    stImg.virt_addr = (char *)pFrame->stImageFrame.pVirAddr;
	stImg.format = pFrame->stVideoHeader.eFormatType;
	stImg.height = (int)pFrame->stVideoHeader.u32ImageHeight;
	stImg.height_stride = pFrame->stVideoHeader.iHeightStride;
	stImg.width = (int)pFrame->stVideoHeader.u32ImageWidth;
	stImg.width_stride = pFrame->stVideoHeader.iWidthStride;
	// 验证图像参数，如果不满足要求则设置合理默认值
	if (!stImg.virt_addr) {
        LOG_ERROR("Image virtual address is null, skipping OSD operation\n");
        return 0; // 跳过OSD操作但返回成功，因为这不是致命错误
    }   
    // 初始化OSD数据
    initializeOsdData(&stOsd, &stImg);
    
    if (!stOsd.osdimg.virt_addr) {
        LOG_ERROR("Failed to initialize OSD data\n");
        return -1;
    }
    
    // 获取RGA实例并执行OSD操作
    RKrga * rkrga = Media_GetRkrga();
    if (!rkrga) {
        LOG_ERROR("Failed to get RGA instance\n");
        free(stOsd.osdimg.virt_addr);  // 释放分配的内存
        return -1;
    }
    
    // 执行RGA OSD操作
    bool result = rkrga->img_osd_virt(&stImg, &stOsd);
    
    if (!result) {
        // 不再记录错误，改为警告，因为在某些平台上OSD功能不可用是正常的
        LOG_DEBUG("RGA OSD operation not supported on this platform, skipping OSD overlay\n");
        free(stOsd.osdimg.virt_addr);  // 如果操作失败，则释放分配的内存
        // 返回成功，因为OSD不是关键功能，不应该因为缺少OSD导致编码失败
        return 0;
    }
        
    // 成功后释放OSD图像内存
    free(stOsd.osdimg.virt_addr);
    
    return 0;
}	
INT32 MediaOsd::MediaOsdOpenCvSetTime(MEDIA_VIDEO_FRAME_T *pFrame)
{
    if (!pFrame || !pFrame->stImageFrame.pVirAddr) {
        LOG_ERROR("Invalid input frame or virtual address is null\n");
        return -1;
    }

    // 获取系统时间
    char timeStr[64];
    createTimeString(timeStr, sizeof(timeStr));

    // 根据图像格式创建OpenCV Mat对象
    int width = pFrame->stVideoHeader.u32ImageWidth;
    int height = pFrame->stVideoHeader.u32ImageHeight;
    int width_stride = pFrame->stVideoHeader.iWidthStride;
    int format = pFrame->stVideoHeader.eFormatType;

    try {
        // 根据不同的图像格式创建Mat对象
        if (format == MEDIA_FORMAT_YUV420SP) { // NV12格式
            // 从NV12格式的原始数据创建BGR Mat，进行处理后再转回NV12
            unsigned char *src_ptr = (unsigned char*)pFrame->stImageFrame.pVirAddr;
            
            // 创建临时NV12 Mat
            cv::Mat src_nv12(height * 3/2, width_stride, CV_8UC1);
            
            // 复制原始数据到临时Mat
            for (int i = 0; i < height; ++i) {
                memcpy(src_nv12.data + i * width_stride, 
                        src_ptr + i * width_stride, 
                        width);
            }
            
            // 复制UV平面数据
            for (int i = 0; i < height / 2; ++i) {
                memcpy(src_nv12.data + (height + i) * width_stride, 
                        src_ptr + (height + i) * width_stride, 
                        width);
            }
            
            // 转换到BGR进行绘图
            cv::Mat bgr_mat;
            cv::cvtColor(src_nv12, bgr_mat, cv::COLOR_YUV2BGR_NV21, 3);
            
            // 绘制时间戳
            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.7;
            cv::Scalar color(0, 255, 0); // 绿色
            int thickness = 2;
            int baseline = 0;

            // 计算文本尺寸
            cv::Size textSize = cv::getTextSize(timeStr, fontFace, fontScale, thickness, &baseline);
            baseline += thickness;

            // 设置文本位置（右上角，距离边缘10像素）
            int x = width - textSize.width - 10;
            int y = textSize.height + 10;

            // 绘制文字
            cv::putText(bgr_mat, timeStr, cv::Point(x, y), fontFace, fontScale, color, thickness);

            // 转换回NV12格式
            cv::Mat dst_nv12;
            cv::cvtColor(bgr_mat, dst_nv12, cv::COLOR_BGR2YUV_I420);
            
            // 将处理后的数据复制回原始帧
            unsigned char *dst_ptr = (unsigned char*)pFrame->stImageFrame.pVirAddr;

            // 复制Y平面数据
            for (int i = 0; i < height; ++i) {
                memcpy(dst_ptr + i * width_stride, 
                        dst_nv12.data + i * width, 
                        width);
            }

            // 复制UV平面数据
            unsigned char *uv_src = dst_nv12.data + width * height;  // UV数据起始位置
            unsigned char *uv_dst = dst_ptr + height * width_stride;  // 目标UV数据起始位置
            
            for (int i = 0; i < height / 2; ++i) {
                // NV12格式中UV是交错存储的，需要特殊处理
                for (int j = 0; j < width; j += 2) {
                    if ((j + 1) < width) {
                        // 将U和V值交替存储
                        uv_dst[i * width_stride + j] = uv_src[i * width + j];      // U
                        uv_dst[i * width_stride + j + 1] = uv_src[i * width + j + 1]; // V
                    } else {
                        // 如果宽度是奇数，单独处理最后一个像素
                        uv_dst[i * width_stride + j] = uv_src[i * width + j];
                    }
                }
            }
        } else if (format == MEDIA_FORMAT_ARGB8888) { // ARGB格式
            cv::Mat img(height, width, CV_8UC4, pFrame->stImageFrame.pVirAddr);
            int x = width - 150; // 距离右边一定距离
            int y = 30; // 距离顶部一定距离

            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.7;
            cv::Scalar color(0, 255, 0, 255); // 绿色
            int thickness = 2;

            cv::putText(img, timeStr, cv::Point(x, y), fontFace, fontScale, color, thickness);
        } else if (format == MEDIA_FORMAT_ABGR8888) { // ABGR格式
            cv::Mat img(height, width, CV_8UC4, pFrame->stImageFrame.pVirAddr);
            int x = width - 150; // 距离右边一定距离
            int y = 30; // 距离顶部一定距离

            int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            double fontScale = 0.7;
            cv::Scalar color(0, 255, 0, 255); // 绿色
            int thickness = 2;

            cv::putText(img, timeStr, cv::Point(x, y), fontFace, fontScale, color, thickness);
        } else {
            LOG_DEBUG("Unsupported format for OpenCV OSD: %d, skipping overlay\n", format);
            return 0;
        }

        LOG_DEBUG("OpenCV OSD timestamp overlay completed successfully\n");
        return 0;
    } catch (const cv::Exception& e) {
        LOG_ERROR("OpenCV exception during OSD operation: %s\n", e.what());
        return -1;
    } catch (...) {
        LOG_ERROR("Unknown exception during OpenCV OSD operation\n");
        return -1;
    }
}
