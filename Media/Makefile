MODULE_CUR_PATH = $(CURDIR)
MODULE_SUB_DIR = src
MODULE_SUB_INC = inc
MODULE_SUB_INC += src/include
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Config/inc

MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Utils/inc
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Hal/inc/Base
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Hal/inc/System
ifeq ($(TARGET_MEDIA),mpp)
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Libs/$(SOC)/mpp/include
MPP_LIB_PATH = $(MODULE_CUR_PATH)/../Libs/$(SOC)/mpp/lib
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Libs/$(SOC)/rga/include
RGA_LIB_PATH = $(MODULE_CUR_PATH)/../Libs/$(SOC)/rga/lib
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Libs/$(SOC)/npu/include
NPU_LIB_PATH = $(MODULE_CUR_PATH)/../Libs/$(SOC)/npu/lib
#-ldl -lm -lpthread -lrt --std=c99 -D _GNU_SOURCE
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Werror
LD_C_FLAGS += -L$(MPP_LIB_PATH) -L$(RGA_LIB_PATH) -L$(NPU_LIB_PATH) -DMEDIARKMPP
LD_CPP_FLAGS += $(WARNING) -L$(MPP_LIB_PATH) -L$(NPU_LIB_PATH) -DMEDIARKMPP
MODULE_SUB_INC += src/rockchip/inc
MODULE_SUB_DIR += src/rockchip/src
else ifeq ($(TARGET_MEDIA),hdal)
LD_C_FLAGS += -DHDAL_DEF
LD_CPP_FLAGS += -DHDAL_DEF
endif

#libv4l2
ifneq (,$(findstring y,$(TARGET_V4L2)))
MODULE_SUB_DIR += src/libv4l2/src
MODULE_SUB_INC += src/libv4l2/inc
endif

#App use audio
ifneq (,$(findstring y,$(TARGET_AUDIO)))
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Libs/$(SOC)/alsa/include
MODULE_SUB_INC += $(MODULE_CUR_PATH)/../Libs/$(SOC)/fdkacc/include
LD_C_FLAGS += -DUSE_AUDIO
LD_CPP_FLAGS += -DUSE_AUDIO
endif

#App use opencv
ifneq (,$(findstring y,$(TARGET_OPENCV)))
MODULE_SUB_INC += $(CURDIR)/../Libs/opencv/include
OPENCV_LIB_PATH = $(CURDIR)/../Libs/opencv/lib
LD_C_FLAGS += -L$(OPENCV_LIB_PATH) -DUSE_OPENCV
LD_CPP_FLAGS += -L$(OPENCV_LIB_PATH) -DUSE_OPENCV
endif

MODULE_NAME = Media
LINK_SHARE=y
include $(ROOT_PATH)/Rules/platform.mk
#模块库mk  调试执行程序mk
include $(ROOT_PATH)/Rules/module.mk