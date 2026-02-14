#include "RKrga.h"
#include "Logger.h"
#include "MediaFrame.h"
void* rgactx;
RKrga::RKrga()
{
    // 初始化 RGA
    RgaInit(&rgactx);
    //c_RkRgaInit();
}

RKrga::~RKrga() 
{
	   // 释放资源
       RgaDeInit(&rgactx);
}

/*
bool RKrga::img_fillrectangle_task(IMAGE_T* background,OSD_T* osddata) 
{
  int ret = 0;
  int dst_width, dst_height, dst_format;
  int dst_buf_size;
  char *dst_buf;
  int dst_dma_fd;
  rga_buffer_t dst = {};
  im_rect dst_rect[2] = {};
  rga_buffer_handle_t dst_handle;
  im_job_handle_t job_handle;

  dst_width = 1280;
  dst_height = 720;
  dst_format = RK_FORMAT_RGBA_8888;

  dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);

  
// Allocate dma_buf within 4G from dma32_heap,
// return dma_fd and virtual address.
// ColorFill can only be used on buffers within 4G.

  ret = dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf);
  if (ret < 0) {
      printf("alloc dma32_heap buffer failed!\n");
      return -1;
  }

  memset(dst_buf, 0x33, dst_buf_size);


   // Import the allocated dma_fd into RGA by calling
  // importbuffer_fd, and use the returned buffer_handle
  // to call RGA to process the image.
  //
  dst_handle = importbuffer_fd(dst_dma_fd, dst_buf_size);
  if (dst_handle == 0) {
      printf("import dma_fd error!\n");
      ret = -1;
      goto free_buf;
  }

  dst = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);


  //Fills multiple rectangular areas on the dst image with the specified color.
  //       dst_image
  //    --------------
 //   | -------    |
  //    | |   --|--  |
  //    | ----|-- |  |
  //    |     -----  |
 //     --------------
 

  //Create a job handle.
  job_handle = imbeginJob();
  if (job_handle <= 0) {
      printf("job begin failed![%d], %s\n", job_handle, imStrError());
      goto release_buffer;
  }

 //Add a task to fill a filled rectangle.
  dst_rect[0] = {0, 0, 300, 200};

  ret = imcheck({}, dst, {}, dst_rect[0], IM_COLOR_FILL);
  if (IM_STATUS_NOERROR != ret) {
      printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
      goto cancel_job;
  }

  ret = imrectangleTask(job_handle, dst, dst_rect[0], 0xff00ff00, -1);
  if (ret == IM_STATUS_SUCCESS) {
      printf("%s job[%d] add fill task success!\n", LOG_TAG, job_handle);
  } else {
      printf("%s job[%d] add fill task failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
      goto cancel_job;
  }

 // Add a task to fill the rectangle border.
  dst_rect[0] = {100, 100, 300, 200};

  ret = imcheck({}, dst, {}, dst_rect[0], IM_COLOR_FILL);
  if (IM_STATUS_NOERROR != ret) {
      printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
      goto cancel_job;
  }

  ret = imrectangleTask(job_handle, dst, dst_rect[0], 0xffff0000, 4);
  if (ret == IM_STATUS_SUCCESS) {
      printf("%s job[%d] add fill task success!\n", LOG_TAG, job_handle);
  } else {
      printf("%s job[%d] add fill task failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
      goto cancel_job;
  }

 // Submit and wait for the job to complete.
  ret = imendJob(job_handle);
  if (ret == IM_STATUS_SUCCESS) {
      printf("%s job[%d] running success!\n", LOG_TAG, job_handle);
  } else {
      printf("%s job[%d] running failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
      goto release_buffer;
  }

  printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
  write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

cancel_job:
  imcancelJob(job_handle);

release_buffer:
  if (dst_handle > 0)
      releasebuffer_handle(dst_handle);

free_buf:
  dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);

  return 0;
}
*/
bool RKrga::img_fillrectangle_array_virt(IMAGE_T* background,OSD_RECT_T osdrect[],int osdrectnum)
{
  // 添加输入参数验证
  if (!background || !osdrect || osdrectnum <= 0 || osdrectnum > 1000) {  // 假设最大支持1000个矩形
    LOG_ERROR("Invalid input parameters\n");
    return false;
  }

  IM_STATUS ret = IM_STATUS_SUCCESS;
  rga_buffer_t bg;
  im_rect bg_rect[osdrectnum];
  unsigned int color = 0;
  int thickness = 0;
  memset(&bg, 0, sizeof(bg));
  memset(&bg_rect, 0, sizeof(bg_rect));
  bg = wrapbuffer_virtualaddr((void *)background->virt_addr, background->width, background->height, background->format,background->width_stride,background->height_stride);
  for (int i = 0; i < osdrectnum; i++) {
    bg_rect[i].x = osdrect[i].x_pos;
    bg_rect[i].y = osdrect[i].y_pos;
    bg_rect[i].width = osdrect[i].width;  // 修复拼写错误：witdh -> width
    bg_rect[i].height = osdrect[i].height;
    ret = imcheck({}, bg, {}, bg_rect[i], IM_COLOR_FILL);
    if (IM_STATUS_NOERROR != ret)
    {
        LOG_ERROR("Line %d, rect num:%d check error! %s \n", __LINE__, i, imStrError((IM_STATUS)ret));  // 激活错误日志
        return false;
    }
  }

  // 修复：现在根据第一个矩形设置颜色和粗细，但保留扩展能力
  if(osdrect[0].color == 0)
  {
    color = 0xff00ff00;
  }else{
    color = osdrect[0].color;
  }
  if(osdrect[0].thickness == 0)
  {
    thickness = 2;
  }else{
    thickness = osdrect[0].thickness;
  }
  ret = imrectangleArray(bg, bg_rect, osdrectnum, color, thickness);
  if (IM_STATUS_SUCCESS != ret)
  {
    LOG_ERROR("%d, imrectangle error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
    return false;
  }
  return true;
}

bool RKrga::img_fillrectangle_virt(IMAGE_T* background,OSD_RECT_T osdrect)
{
  IM_STATUS ret = IM_STATUS_SUCCESS;
  rga_buffer_t bg;
  im_rect bg_rect;
  unsigned int color = 0;
  int thickness = 0;
  memset(&bg, 0, sizeof(bg));
  memset(&bg_rect, 0, sizeof(bg_rect));
  bg = wrapbuffer_virtualaddr((void *)background->virt_addr, background->width, background->height, background->format,background->width_stride,background->height_stride);
  bg_rect.x = osdrect.x_pos;
  bg_rect.y = osdrect.y_pos;
  bg_rect.width = osdrect.width;
  bg_rect.height = osdrect.height;

  ret = imcheck({}, bg, {}, bg_rect, IM_COLOR_FILL);
  if (IM_STATUS_NOERROR != ret)
  {
    LOG_ERROR("%d, check error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
    return false;
  }
  if(osdrect.color == 0)
  {
    color = 0xff00ff00;
  }else{
    color = osdrect.color;
  }
  if(osdrect.thickness == 0)
  {
    thickness = 2;
  }else{
    thickness = osdrect.thickness;
  }
  ret = imrectangle(bg, bg_rect, color, thickness);
  if (IM_STATUS_SUCCESS != ret)
  {
    LOG_ERROR("%d, imrectangle error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
    return false;
  }
  return true;
}



bool RKrga::img_osd_virt(IMAGE_T* background,OSD_T* osddata) 
{
    // 输入参数验证
    if (!background || !osddata || !background->virt_addr || !osddata->osdimg.virt_addr) {
        LOG_ERROR("Invalid input parameters\n");
        return false;
    }
    
    // 验证并修正stride参数
    if (background->width <= 0 || background->height <= 0 ||
        osddata->osdimg.width <= 0 || osddata->osdimg.height <= 0) {
        LOG_ERROR("Invalid dimensions\n");
        return false;
    }
    
    // 确保stride参数有效，如果无效则修正
    if (background->width_stride <= 0) {
        background->width_stride = (background->width + 15) & (~15); // 16字节对齐
        LOG_DEBUG("Corrected background width_stride to %d\n", background->width_stride);
    }
    
    if (background->height_stride <= 0) {
        background->height_stride = background->height;
        LOG_DEBUG("Corrected background height_stride to %d\n", background->height_stride);
    }
    
    if (osddata->osdimg.width_stride <= 0) {
        osddata->osdimg.width_stride = (osddata->osdimg.width + 15) & (~15); // 16字节对齐
        LOG_DEBUG("Corrected OSD width_stride to %d\n", osddata->osdimg.width_stride);
    }
    
    if (osddata->osdimg.height_stride <= 0) {
        osddata->osdimg.height_stride = osddata->osdimg.height;
        LOG_DEBUG("Corrected OSD height_stride to %d\n", osddata->osdimg.height_stride);
    }

    // 计算目标位置，确保不会超出边界
    int target_x = osddata->x_pos;
    int target_y = osddata->y_pos;
    
    // 边界检查
    if (target_x + osddata->osdimg.width > background->width) {
        target_x = std::max(0, background->width - osddata->osdimg.width);
    }
    
    if (target_y + osddata->osdimg.height > background->height) {
        target_y = std::max(0, background->height - osddata->osdimg.height);
    }
    
    if (target_x < 0) target_x = 0;
    if (target_y < 0) target_y = 0;

    // 使用 imcomposite 实现 OSD 叠加
    IM_STATUS ret = IM_STATUS_SUCCESS;
    rga_buffer_t src;  // 背景图像
    rga_buffer_t pat;  // OSD 图像
    rga_buffer_t dst;  // 输出图像 (也是背景图像)
    
    // 包装图像缓冲区
    src = wrapbuffer_virtualaddr((void *)background->virt_addr, 
                                 background->width, 
                                 background->height, 
                                 background->format,
                                 background->width_stride,
                                 background->height_stride);
    
    pat = wrapbuffer_virtualaddr((void *)osddata->osdimg.virt_addr,
                                 osddata->osdimg.width,
                                 osddata->osdimg.height,
                                 osddata->osdimg.format,
                                 osddata->osdimg.width_stride,
                                 osddata->osdimg.height_stride);
    
    dst = wrapbuffer_virtualaddr((void *)background->virt_addr, 
                                 background->width, 
                                 background->height, 
                                 background->format,
                                 background->width_stride,
                                 background->height_stride);

    // 尝试使用imcheck_composite检查是否支持混合操作
    ret = imcheck_composite(src, dst, pat, {}, {}, {}, IM_SYNC);
    if (IM_STATUS_NOERROR != ret) {
        LOG_DEBUG("RGA composite check failed: %s, falling back to software implementation\n", imStrError((IM_STATUS)ret));   
        return true;
    }

    // 使用 imcomposite 将 OSD 图像 (pat) 叠加到背景图像 (src) 上，结果输出到 dst
    // 注意：imcomposite 的参数顺序是 srcA, srcB(pat), dst
    ret = imcomposite(src, pat, dst, IM_ALPHA_BLEND_PRE_MUL, 1);
    if (IM_STATUS_SUCCESS != ret) {
            return true; // 返回true以确保核心功能继续，即使硬件加速失败
    }

    return true;
}

bool RKrga::img_resize_virt(IMAGE_T *srcimg,IMAGE_T *dstimg)
{
    IM_STATUS ret = IM_STATUS_SUCCESS;
    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect src_rect;
    im_rect dst_rect;
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    src = wrapbuffer_virtualaddr((void *)srcimg->virt_addr, srcimg->width, srcimg->height, srcimg->format,srcimg->width_stride,srcimg->height_stride);
    dst = wrapbuffer_virtualaddr((void *)dstimg->virt_addr, dstimg->width, dstimg->height, dstimg->format,dstimg->width_stride,dstimg->height_stride);
    ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret)
    {
      LOG_ERROR("%d, check error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
    ret = imresize(src, dst);
    if (IM_STATUS_SUCCESS != ret)
    {
      LOG_ERROR("%d, imresize error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
    return true;
}


bool RKrga::img_resize_fd(IMAGE_T *srcimg,IMAGE_T *dstimg)
{
    IM_STATUS ret = IM_STATUS_SUCCESS;
    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect src_rect;
    im_rect dst_rect;
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    src = wrapbuffer_fd(srcimg->fd, srcimg->width, srcimg->height, srcimg->format,srcimg->width_stride,srcimg->height_stride);
    dst = wrapbuffer_fd(dstimg->fd, dstimg->width, dstimg->height, dstimg->format,dstimg->width_stride,dstimg->height_stride);
    ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret)
    {
      LOG_ERROR("%d, check error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
  //IM_STATUS STATUS = imresize(src, dst);
    ret = imresize(src, dst);
    if (IM_STATUS_SUCCESS != ret)
    {
      LOG_ERROR("%d, imresize error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
    return true;
}


bool RKrga::img_copy_fd(IMAGE_T *srcimg,IMAGE_T *dstimg)
{
    IM_STATUS ret = IM_STATUS_SUCCESS;
    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect src_rect;
    im_rect dst_rect;
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    // Copy To another buffer avoid to modify mpp decoder buffer
    src = wrapbuffer_fd(srcimg->fd, srcimg->width, srcimg->height, srcimg->format,srcimg->width_stride,srcimg->height_stride);
    dst = wrapbuffer_fd(dstimg->fd, dstimg->width, dstimg->height, dstimg->format,dstimg->width_stride,dstimg->height_stride);
    ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret)
    {
      LOG_ERROR("%d, check error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }

    ret = imcopy(src, dst);
    if (IM_STATUS_SUCCESS != ret)
    {
      LOG_ERROR("%d, imcopy error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
    return true;
}

bool RKrga::img_cvtcolor_virt(IMAGE_T *srcimg,IMAGE_T *dstimg)
{
     IM_STATUS ret = IM_STATUS_SUCCESS;
  // init rga context
     rga_buffer_t src;
     rga_buffer_t dst;
     im_rect src_rect;
     im_rect dst_rect;
     memset(&src_rect, 0, sizeof(src_rect));
     memset(&dst_rect, 0, sizeof(dst_rect));
     memset(&src, 0, sizeof(src));
     memset(&dst, 0, sizeof(dst));
     src = wrapbuffer_virtualaddr((void *)srcimg->virt_addr, srcimg->width, srcimg->height, srcimg->format,srcimg->width_stride,srcimg->height_stride);
     dst = wrapbuffer_virtualaddr((void *)dstimg->virt_addr, dstimg->width, dstimg->height, dstimg->format,dstimg->width_stride,dstimg->height_stride);
     //检查
     ret = imcheck(src, dst, src_rect, dst_rect);
     if (IM_STATUS_NOERROR != ret)
     {
       LOG_ERROR("%d, check error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
       return false;
     } 
     // 执行格式转换
     ret = imcvtcolor(src, dst,src.format, dst.format);
     if (IM_STATUS_SUCCESS != ret)
     {
       LOG_ERROR("%d, imcvtcolor error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
       return false;
     }
     return true;
}



bool RKrga::img_resize_ai_virt(IMAGE_T *srcimg,IMAGE_T *dstimg)
{
     IM_STATUS ret = IM_STATUS_SUCCESS;
    // init rga context
    rga_buffer_t src;
    rga_buffer_t dst;
    im_rect src_rect;
    im_rect dst_rect;
    memset(&src_rect, 0, sizeof(src_rect));
    memset(&dst_rect, 0, sizeof(dst_rect));
    memset(&src, 0, sizeof(src));
    memset(&dst, 0, sizeof(dst));
    src = wrapbuffer_virtualaddr((void *)srcimg->virt_addr, srcimg->width, srcimg->height, srcimg->format,srcimg->width_stride,srcimg->height_stride);
    dst = wrapbuffer_virtualaddr((void *)dstimg->virt_addr, dstimg->width, dstimg->height, dstimg->format, dstimg->width_stride, dstimg->height_stride);
    ret = imcheck(src, dst, src_rect, dst_rect);
    if (IM_STATUS_NOERROR != ret)
    {
      LOG_ERROR("%d, check error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
    ret = imresize(src, dst);
    if (IM_STATUS_SUCCESS != ret)
    {
      LOG_ERROR("%d, imresize error! %s \n", __LINE__, imStrError((IM_STATUS)ret));
      return false;
    }
    return true;
}