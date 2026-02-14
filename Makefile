VIDEOPIPOVISION_VERSION = 1.0.0
# common
CURDIR = $(shell pwd)
export ROOT_PATH = $(CURDIR)

#misc
BUILD_COMPLETE_STRING ?= $(shell date "+%a, %d %b %Y %T %z")
UID := $(shell id -u)
GID := $(shell id -g)


OUTPUT_DIR= $(CURDIR)/output


# media view
#tsp config
TARGET_MEDIA = mpp
TARGET_SOC = rk3566
TARGET_OPENCV = y
TARGET_ROCKCHIP = y
TARGET_ROCKCHIP_FFMPEG = y
TARGET_V4L2 = y
TARGET_AUDIO = y
TARGET_LVGL = y
TARGET_LVGL_DEMO = n
TARGET_AI = y
# cross host
TARGET_CROSS_HOST = $(ROOT_PATH)/../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu//bin/aarch64-linux-gnu
#TARGET_CROSS_HOST = $(ROOT_PATH)/../prebuilts/gcc/linux-x86/aarch64/gcc-buildroot-9.3.0-2020.03-x86_64_aarch64-rockchip-linux-gnu/bin/aarch64-rockchip-linux-gnu
#tsp end

#lubanmav3
#TARGET_MEDIA = mpp
#TARGET_SOC = rk3576
#TARGET_OPENCV = y
#TARGET_ROCKCHIP = y
#TARGET_ROCKCHIP_FFMPEG = y
#TARGET_V4L2 = y
#TARGET_ALSA = n
#TARGET_CROSS_HOST = $(ROOT_PATH)/../prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu
include $(ROOT_PATH)/Config/Target.mk
export SOC     := $(TARGET_SOC)
export MEDIATYPE   := $(TARGET_MEDIA)
export OPENCV   :=$(TARGET_OPENCV)
export ROCKCHIP :=$(TARGET_ROCKCHIP)
# cross host


export $(TARGET_CROSS_HOST)

# rules
include $(ROOT_PATH)/linux.mk

#path
APP_DIR=$(ROOT_PATH)/App
HARDWARE_DIR = $(ROOT_PATH)/Hardware
LIB_DIR=$(ROOT_PATH)/Libs
MEDIA_DIR = $(ROOT_PATH)/Media
UTILS_DIR = $(ROOT_PATH)/Utils
HAL_DIR = $(ROOT_PATH)/Hal


CROSS_COMPILE=$(TARGET_CROSS_HOST)- 

LD_C_FLAGS   +=  -ldl -lm -lpthread -lrt  -std=c99
LD_CPP_FLAGS +=  -ldl -lm -lpthread -lrt  -lstdc++  #C++参数 

LIB_VAR=Utils
ifneq (,$(findstring y,$(TARGET_OPENCV)))
LIB_VAR += opencv_calib3d
LIB_VAR += opencv_core
LIB_VAR += opencv_dnn
LIB_VAR += opencv_features2d
LIB_VAR += opencv_flann
LIB_VAR += opencv_highgui
LIB_VAR += opencv_imgcodecs
LIB_VAR += opencv_imgproc
LIB_VAR += opencv_ml
LIB_VAR += opencv_objdetect
LIB_VAR += opencv_photo
LIB_VAR += opencv_stitching
LIB_VAR += opencv_video
LIB_VAR += opencv_videoio
endif
ifneq (,$(findstring y,$(TARGET_ROCKCHIP)))
LIB_VAR += rga
LIB_VAR += rockchip_mpp
LIB_VAR += rockchip_vpu
LIB_VAR += rknnrt
LD_CPP_FLAGS += -DMEDIARKMPP
LIB_VAR += mp4v2
endif
ifneq (,$(findstring y,$(TARGET_ROCKCHIP_FFMPEG)))
LIB_VAR += drm_amdgpu
LIB_VAR += drm_etnaviv
LIB_VAR += drm_freedreno
LIB_VAR += drm_nouveau
LIB_VAR += drm_radeon
LIB_VAR += drm
LIB_VAR += x264
LIB_VAR += avformat
LIB_VAR += avdevice
LIB_VAR += avcodec
LIB_VAR += avutil
LIB_VAR += avfilter
LIB_VAR += postproc
LIB_VAR += swresample
LIB_VAR += swscale
#LIB_VAR += gbm
#LIB_VAR += EGL
endif
ifneq (,$(findstring y,$(TARGET_AUDIO)))
LIB_VAR += asound
LIB_VAR += fdk-aac
endif

LIB_VAR += Media
LIB_VAR += App
LIB_VAR += Hardware
LIB_VAR += Hal
PROJECT_LIB_VAR=$(LIB_VAR)
ifneq ($(PROJECT_LIB_VAR), "")
PROJECTLIB_VAR += $(patsubst %,-l%,$(PROJECT_LIB_VAR))
endif

INC_PATH = $(APP_DIR)/inc
INC_PATH += $(MEDIA_DIR)/inc
INC_PATH += $(UTILS_DIR)/inc
INC_PATH += $(HAL_DIR)/inc
INC_PATH += $(HARDWARE_DIR)/inc
ifneq (,$(findstring y,$(TARGET_ROCKCHIP)))
INC_PATH += $(LIB_DIR)/$(TARGET_SOC)/mpp/include
INC_PATH += $(LIB_DIR)/$(TARGET_SOC)/rga/include
INC_PATH += $(LIB_DIR)/$(TARGET_SOC)/npu/include
endif
ifneq (,$(findstring y,$(TARGET_OPENCV)))
INC_PATH += $(LIB_DIR)/opencv/include
endif
ifneq (,$(findstring y,$(TARGET_ROCKCHIP_FFMPEG)))
INC_PATH += $(LIB_DIR)/$(TARGET_SOC)/ffmpeg/include
INC_PATH += $(LIB_DIR)/$(TARGET_SOC)/x264/include
endif
ifneq (,$(findstring y,$(TARGET_AUDIO)))
INC_PATH += $(LIB_DIR)/$(TARGET_SOC)/alsa/include
endif

PROJECT_INC_PATH += $(INC_PATH)

ifneq ($(PROJECT_INC_PATH), "")
PROJECT_INC_DIRS = $(shell find $(PROJECT_INC_PATH) -maxdepth 0 -type d)
PROJECTINC_PATH += $(patsubst %,-I%,$(PROJECT_INC_DIRS))
endif
BUILD_ALL = Utils
BUILD_ALL += Hal
BUILD_ALL += Hardware
BUILD_ALL += Media
BUILD_ALL += App


.PHONY: App App_Clean  Media  Media_Clean Utils Utils_Clean Hardware Hardware_Clean Hal Hal_Clean

all: $(BUILD_ALL)
	$(CXX) -o $(OUTPUT_DIR)/VideoVision  $(PROJECTINC_PATH) -L $(OUTPUT_DIR)/lib/ $(PROJECTLIB_VAR) $(LD_CPP_FLAGS) main.cpp
	$(RM)  *.o -rf
	$(CP) $(LIB_DIR)/$(SOC)/mpp/bin/mpi_enc_test $(OUTPUT_DIR)/
	$(CP) $(LIB_DIR)/$(SOC)/ffmpeg/bin/ffmpeg $(OUTPUT_DIR)/
	$(ECHO) "Finish generating images at $(BUILD_COMPLETE_STRING)"
	
pack:
	$(CXX) -o $(OUTPUT_DIR)/VideoVision  $(PROJECTINC_PATH) -L $(OUTPUT_DIR)/lib/ $(PROJECTLIB_VAR) $(LD_CPP_FLAGS) main.cpp
	$(RM)  *.o -rf
	$(CP) $(LIB_DIR)/$(SOC)/mpp/bin/mpi_enc_test $(OUTPUT_DIR)/
	$(CP) $(LIB_DIR)/$(SOC)/ffmpeg/bin/ffmpeg $(OUTPUT_DIR)/
	$(ECHO) "Finish pack images at $(BUILD_COMPLETE_STRING)"

checkenv:
	@if [ ! -e $(OUTPUT_DIR)/lib ]; then \
		mkdir $(OUTPUT_DIR); \
		mkdir $(OUTPUT_DIR)/lib; \
	fi
	$(CP) $(ROOT_PATH)/Config/lv_conf.h $(UTILS_DIR)/Thirdparty/lvgl/
	find $(LIB_DIR)/$(SOC) -name "*.so*" |xargs -i cp {} $(OUTPUT_DIR)/lib/
	$(CP) $(LIB_DIR)/$(SOC)/initlink.sh $(OUTPUT_DIR)/


Utils: checkenv
	@$(ECHO) "##### Build utils ####"
	@make -C $(UTILS_DIR)  TARGET_LVGL=$(TARGET_LVGL) TARGET_LVGL_DEMO=$(TARGET_LVGL_DEMO)
	@if [ -f $(UTILS_DIR)/Lib/libUtils.so ]; then \
		$(CP) $(UTILS_DIR)/Lib/libUtils.so $(OUTPUT_DIR)/lib/; \
	fi

Utils_Clean:
	@$(ECHO) "##### Build utils clean ####"
	make -C $(UTILS_DIR) clean

Hardware: checkenv
	@$(ECHO) "##### Build Hardware ####"
	@make -C $(HARDWARE_DIR)
	@if [ -f $(HARDWARE_DIR)/Lib/libHardware.so ]; then \
		$(CP) $(HARDWARE_DIR)/Lib/libHardware.so $(OUTPUT_DIR)/lib/; \
	fi

Hardware_Clean:
	@$(ECHO) "##### Build Hardware clean ####"
	make -C $(HARDWARE_DIR) clean

Hal: checkenv
	@$(ECHO) "##### Build Hal ####"
	@make -C $(HAL_DIR) TARGET_LVGL=$(TARGET_LVGL) 
	@if [ -f $(HAL_DIR)/Lib/libHal.so ]; then \
		$(CP) $(HAL_DIR)/Lib/libHal.so $(OUTPUT_DIR)/lib/; \
	fi

Hal_Clean:
	@$(ECHO) "##### Build Hal clean ####"
	make -C $(HAL_DIR) clean

App: checkenv
	@$(ECHO) "##### Build app ####"
	@make -C $(APP_DIR) TARGET_MEDIA=$(TARGET_MEDIA) TARGET_OPENCV=$(TARGET_OPENCV) TARGET_FFMPEG=$(TARGET_ROCKCHIP_FFMPEG) TARGET_AUDIO=$(TARGET_AUDIO) TARGET_LVGL=$(TARGET_LVGL) TARGET_LVGL_DEMO=$(TARGET_LVGL_DEMO) TARGET_AI=$(TARGET_AI)
	@if [ -f $(APP_DIR)/Lib/libApp.so ]; then \
		$(CP) $(APP_DIR)/Lib/libApp.so $(OUTPUT_DIR)/lib/; \
	fi

App_Clean:
	@$(ECHO) "##### Build app clean ####"
	make -C $(APP_DIR) clean

Media: checkenv
	@$(ECHO) "##### Build Media ####"
	@make -C $(MEDIA_DIR) TARGET_MEDIA=$(TARGET_MEDIA) TARGET_V4L2=$(TARGET_V4L2) TARGET_AUDIO=$(TARGET_AUDIO) TARGET_OPENCV=$(TARGET_OPENCV)
	@if [ -f $(MEDIA_DIR)/Lib/libMedia.so ]; then \
		$(CP) $(MEDIA_DIR)/Lib/libMedia.so $(OUTPUT_DIR)/lib/; \
	fi

Media_Clean: 
	@$(ECHO) "##### Build media clean ####"
	make -C $(MEDIA_DIR) clean

clean: App_Clean Media_Clean Utils_Clean Hardware_Clean Hal_Clean
	@$(RM)  *.o -rf
	@$(ECHO) "RM  $(OUTPUT_DIR)"
	@$(RM)  $(OUTPUT_DIR)

