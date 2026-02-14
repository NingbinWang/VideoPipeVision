#ifndef _RKRGA_H_
#define _RKRGA_H_
#include <dlfcn.h> 
#include "RgaApi.h"
#include "im2d.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct
{
  int width;
  int height;
  int width_stride;
  int height_stride;
  int format;
  char *virt_addr;
  int fd;
} IMAGE_T;

typedef struct
{
  int block_width;//一个block的大小
  int block_height;
  int block_count;
  int x_pos;
  int y_pos;
  IMAGE_T osdimg;
} OSD_T;

typedef struct
{
  int width;
  int height;
  int x_pos;
  int y_pos;
  unsigned int color;
  int thickness;
} OSD_RECT_T;


typedef int(* FUNC_RGA_INIT)();
typedef void(* FUNC_RGA_DEINIT)();
typedef int(* FUNC_RGA_BLIT)(rga_info_t *, rga_info_t *, rga_info_t *);

typedef struct _rga_context{
    void *rga_handle;
    FUNC_RGA_INIT init_func;
    FUNC_RGA_DEINIT deinit_func;
    FUNC_RGA_BLIT blit_func;
} rga_context;

//int RGA_init(rga_context* rga_ctx);

#ifdef __cplusplus
}
#endif


class RKrga{
 public:
    RKrga();
    ~RKrga();
    bool img_osd_virt(IMAGE_T* background,OSD_T* osddata);
    bool img_resize_virt(IMAGE_T *srcimg,IMAGE_T *dstimg);
    bool img_resize_fd(IMAGE_T *srcimg,IMAGE_T *dstimg);
    bool img_copy_fd(IMAGE_T *srcimg,IMAGE_T *dstimg);
    bool img_cvtcolor_virt(IMAGE_T *srcimg,IMAGE_T *dstimg);
    bool img_fillrectangle_virt(IMAGE_T* background,OSD_RECT_T osdrect);
    bool img_fillrectangle_array_virt(IMAGE_T* background,OSD_RECT_T osdrect[],int osdrectnum);
    bool img_resize_ai_virt(IMAGE_T *srcimg,IMAGE_T *dstimg);
    //bool img_fillrectangle_task(IMAGE_T* background,OSD_T* osddata);
};





#endif