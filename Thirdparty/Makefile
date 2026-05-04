SHELL = /bin/bash
PYTHON := python
CURDIR = $(shell pwd)
#$(shell mkdir -p $(CURDIR)/output)
#export ROOT_PATH = $(CURDIR)/output
export ROOT_PATH = $(CURDIR)
# cross host
#TARGET_CROSS_COMPILE_DIR = $(CURDIR)/../../prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu
#TARGET_CROSS_COMPILE_DIR = $(CURDIR)/../../prebuilts/gcc/linux-x86/aarch64/gcc-buildroot-9.3.0-2020.03-x86_64_aarch64-rockchip-linux-gnu
#TARGET_CROSS_HOST = $(TARGET_CROSS_COMPILE_DIR)/bin/aarch64-linux-gnu
#TARGET_CROSS_HOST = $(TARGET_CROSS_COMPILE_DIR)/bin/aarch64-rockchip-linux-gnu
#TARGET_COMPILE_LIBCDIR=$(TARGET_CROSS_COMPILE_DIR)/aarch64-linux-gnu/libc
#TARGET_COMPILE_LIBCDIR=$(TARGET_CROSS_COMPILE_DIR)/aarch64-rockchip-linux-gnu/lib64
#TARGET_COMPILE_SYSROOT=$(TARGET_CROSS_COMPILE_DIR)/aarch64-rockchip-linux-gnu/sysroot

TARGET_CROSS_COMPILE_DIR = $(CURDIR)/../../prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu

TARGET_CROSS_HOST = $(TARGET_CROSS_COMPILE_DIR)/bin/aarch64-none-linux-gnu

TARGET_COMPILE_LIBCDIR=$(TARGET_CROSS_COMPILE_DIR)/aarch64-none-linux-gnu/libc

export $(TARGET_CROSS_HOST)
##CROSS_COMPILE_HOST = aarch64-linux-gnu
#CROSS_COMPILE_HOST = aarch64-rockchip-linux-gnu

CROSS_COMPILE_HOST = aarch64-none-linux-gnu

ARCH=aarch64
include $(CURDIR)/linux.mk

######WIFI####
WPA_VER=2.9
LIBNL_VER=3.2.25
OPENSSL_VER=3.4.0

#####opencv####
OPENCV_VER=4.10.0
ZLIB_VER=1.3.1
JPEGSRC_VER=v9b
JPEGSRC_DIR=jpeg-9b
LIBPNG_VER=1.6.44
YASM_VER=1.3.0
X264_VER=master
XVID_VER=1.3.7
FFMPEG_VER=4.2.11
YOLOFAST_VER=v.1.1.0
DRM_VER=2.4.124
ALSA_VER=1.2.9
FDKAAC_VER=master
MP4V2_VER=main

#####rockchip####
SOC=rk3566
#SOC=rk3567
MPP_DIR=$(CURDIR)/rkmpp
RGA_DIR=$(CURDIR)/rkrga
FFMPEG_DIR=$(CURDIR)/rkffmpeg
#LIBDRM_DIR=$(CURDIR)/$(SOC)/libdrm-rockchip
 
BUILD_ALL = wpa_supplicant
BUILD_ALL += opencv
#BUILD_INSTALL = i2c_tools_install
#BUILD_INSTALL += iozone3_install
#BUILD_INSTALL += wpa_supplicant_install

BUILD_CLEAN = wpa_supplicant_clean
BUILD_CLEAN += mpp_clean
BUILD_CLEAN += linuxrga_clean
BUILD_CLEAN += ffmpeg_rockchip_clean
BUILD_CLEAN += opencv_clean

all:$(BUILD_ALL)
	@echo "Finish build packages"

install:$(BUILD_INSTALL)
	@echo "Finish install packages"

clean:$(BUILD_CLEAN)
	rm $(ROOT_PATH)/rockchip -rf
	rm $(ROOT_PATH)/opencv -rf
	rm $(ROOT_PATH)/wifi -rf
	@echo "Finish clean packages"

#rockchip
rockchipenv:
	if [ ! -e $(ROOT_PATH)/rockchip ]; then \
		mkdir -p $(ROOT_PATH)/rockchip; \
		mkdir -p $(ROOT_PATH)/rockchip/bin; \
		mkdir -p $(ROOT_PATH)/rockchip/lib; \
		mkdir -p $(ROOT_PATH)/rockchip/include; \
	fi

mpp:rockchipenv
	if [ ! -e $(MPP_DIR) ]; then \
		git clone git@github.com:rockchip-linux/mpp.git $(MPP_DIR); \
	fi
	if [ ! -e $(ROOT_PATH)/rockchip/mpp ]; then \
		touch $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "cmake_minimum_required( VERSION 2.6.3 )" > $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "set(CMAKE_SYSTEM_NAME Linux)" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "set(CMAKE_C_COMPILER $(CC))" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "set(CMAKE_SYSTEM_PROCESSOR \"armv8-a\")" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "set(CMAKE_INSTALL_PREFIX \"$(ROOT_PATH)/rockchip/mpp\")" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "add_definitions(-fPIC)" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "add_definitions(-DARMLINUX)" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		echo "add_definitions(-Dlinux)" >> $(MPP_DIR)/build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ;\
		mkdir -p $(MPP_DIR)/mpp_build ;\
		cd $(MPP_DIR)/mpp_build&&cmake -DCMAKE_BUILD_TYPE=Release -D CMAKE_TOOLCHAIN_FILE=../build/linux/$(ARCH)/$(CROSS_COMPILE_HOST).cmake ../ ;\
		make -C $(MPP_DIR)/mpp_build ;\
		make -C $(MPP_DIR)/mpp_build install ;\
	fi
	cp $(ROOT_PATH)/rockchip/mpp/lib/* $(ROOT_PATH)/rockchip/lib -rf
	cp $(ROOT_PATH)/rockchip/mpp/include/*  $(ROOT_PATH)/rockchip/include -rf
	cp $(ROOT_PATH)/rockchip/mpp/bin/*  $(ROOT_PATH)/rockchip/bin -rf

mpp_clean:
	rm $(ROOT_PATH)/rockchip/mpp -rf
	if [ -e $(MPP_DIR)/mpp_build ]; then \
		make -C $(MPP_DIR)/mpp_build clean; \
		rm $(MPP_DIR)/mpp_build -rf; \
	fi

mpp_install:
	cp $(ROOT_PATH)/rockchip/mpp/lib/* $(CURDIR)/../Libs/$(SOC)/mpp -rf
	cp $(ROOT_PATH)/rockchip/mpp/bin/* $(CURDIR)/../Libs/$(SOC)/mpp/bin -rf
	cp $(ROOT_PATH)/rockchip/mpp/include/* $(CURDIR)/../Libs/$(SOC)/mpp/include -rf

librga:rockchipenv
	if [ ! -e $(RGA_DIR) ]; then \
		wget https://github.com/airockchip/librga/archive/refs/tags/v1.10.0.zip; \
		unzip v1.10.0.zip; \
		rm $(ROOT_PATH)/v1.10.0.zip; \
		mv librga-1.10.0 $(RGA_DIR); \
	fi
	cp 	$(RGA_DIR)/libs/Linux/gcc-$(ARCH)/* $(ROOT_PATH)/rockchip/lib -rf
	cp  $(RGA_DIR)/include/*  $(ROOT_PATH)/rockchip/include -rf

# -Dlibrga_demo=false
linuxrga:rockchipenv
	if [ ! -e $(RGA_DIR) ]; then \
		wget https://github.com/nyanmisaka/rk-mirrors/archive/refs/heads/linux-rga.zip; \
		unzip linux-rga.zip; \
		rm linux-rga.zip; \
		mv rk-mirrors-linux-rga $(RGA_DIR); \
	fi
#	if [ ! -e $(ROOT_PATH)/rockchip/linux-rga ]; then \
#		mkdir -p $(RGA_DIR)/build ;\
#		touch $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "cmake_minimum_required( VERSION 2.6.3 )" > $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake  ;\
#		echo "set(TOOLCHAIN_HOME $(TARGET_CROSS_COMPILE_DIR))" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake  ;\
#		echo "set(CMAKE_SYSTEM_NAME Linux)" >>  $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_C_COMPILER $(CC))" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_INSTALL_PREFIX $(ROOT_PATH)/rockchip/linux-rga)" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_FIND_ROOT_PATH  $(TARGET_CROSS_COMPILE_DIR))" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		echo "set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)" >> $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake ;\
#		mv $(RGA_DIR)/buildroot.cmake $(RGA_DIR)/buildroot.bak; \
#		mv $(RGA_DIR)/build/$(CROSS_COMPILE_HOST).cmake $(RGA_DIR)/buildroot.cmake; \
#		cd $(RGA_DIR)/build&&cmake -DCMAKE_BUILD_TARGET=buildroot ..  ;\
#		make -C $(RGA_DIR)/build  ;\
#		make -C $(RGA_DIR)/build install  ;\
#	fi
	echo "[binaries]"              >  $(ROOT_PATH)/cross_file.txt
	echo "c = '$(CC)'"             >> $(ROOT_PATH)/cross_file.txt
	echo "cpp = '$(CXX)'"          >> $(ROOT_PATH)/cross_file.txt
	echo "ar = '$(AR)'"            >> $(ROOT_PATH)/cross_file.txt
	echo "strip = '$(STRIP)'"      >> $(ROOT_PATH)/cross_file.txt
	echo "pkg-config = '$(CURDIR)/../../prebuilts/gcc/linux-x86/aarch64/gcc-buildroot-9.3.0-2020.03-x86_64_aarch64-rockchip-linux-gnu/bin/pkg-config'"  >> $(ROOT_PATH)/cross_file.txt
	echo "  "                      >> $(ROOT_PATH)/cross_file.txt
	echo "[host_machine]"          >> $(ROOT_PATH)/cross_file.txt
	echo "system = 'linux'"        >> $(ROOT_PATH)/cross_file.txt
	echo "cpu_family = 'aarch64'"  >> $(ROOT_PATH)/cross_file.txt
	echo "cpu = 'aarch64'"         >> $(ROOT_PATH)/cross_file.txt
	echo "endian = 'little'"       >> $(ROOT_PATH)/cross_file.txt
	echo "  "                      >> $(ROOT_PATH)/cross_file.txt
	echo "[properties]"            >> $(ROOT_PATH)/cross_file.txt
	echo "needs_exe_wrapper = true"            >> $(ROOT_PATH)/cross_file.txt
	echo "  "                      >> $(ROOT_PATH)/cross_file.txt
	echo "[built-in options]"            >> $(ROOT_PATH)/cross_file.txt
	echo "c_args = ['-L$(TARGET_CROSS_COMPILE_DIR)/aarch64-rockchip-linux-gnu/sysroot/usr/lib','-L$(TARGET_CROSS_COMPILE_DIR)/lib','-I$(TARGET_CROSS_COMPILE_DIR)/include']"      >> $(ROOT_PATH)/cross_file.txt
	mv $(ROOT_PATH)/cross_file.txt $(RGA_DIR)/
	if [ ! -e $(ROOT_PATH)/rockchip/linux-rga ]; then \
		mkdir -p $(RGA_DIR)/build ;\
		cd $(RGA_DIR) && meson setup -Dprefix=$(ROOT_PATH)/rockchip/linux-rga --libdir=$(ROOT_PATH)/rockchip/lib --buildtype=release --cross-file $(RGA_DIR)/cross_file.txt --default-library=shared -Dcpp_args=-fpermissive -Dlibdrm=false  $(RGA_DIR)/build; \
	    cd $(RGA_DIR) && ninja -C $(RGA_DIR)/build install; \
	fi
#	cp $(ROOT_PATH)/rockchip/linux-rga/lib/* $(ROOT_PATH)/rockchip/lib -rf
	cp $(ROOT_PATH)/rockchip/linux-rga/include/*  $(ROOT_PATH)/rockchip/include -rf
#	cp $(ROOT_PATH)/rockchip/linux-rga/bin/*  $(ROOT_PATH)/rockchip/bin -rf

linuxrga_clean:
	rm $(ROOT_PATH)/rockchip/linux-rga -rf
	if [ -e $(RGA_DIR)/build ]; then \
		make -C $(RGA_DIR)/build clean;\
		rm $(RGA_DIR)/build -rf; \
		rm $(RGA_DIR)/buildroot.cmake; \
	fi


linuxrga_install:
	cp $(ROOT_PATH)/rockchip/linux-rga/lib/* $(CURDIR)/../Libs/$(SOC)/rga -rf
	cp $(ROOT_PATH)/rockchip/linux-rga/bin/* $(CURDIR)/../Libs/$(SOC)/rga/bin -rf
	cp $(ROOT_PATH)/rockchip/linux-rga/include/* $(CURDIR)/../Libs/$(SOC)/rga/include -rf



# --disable-stripping --target-os=linux --cc=$(CC) --enable-libx264 --extra-cflags=-I$(ROOT_PATH)/opencv/x264_install_$(X264_VER)/include/ \
#--extra-ldflags=-L$(ROOT_PATH)/opencv/x264_install_$(X264_VER)/lib/ --enable-rkrga \
	

ffmpeg_rockchip:x264 libdrm mpp librga
	if [ ! -e $(FFMPEG_DIR) ]; then \
		wget https://github.com/nyanmisaka/ffmpeg-rockchip/archive/refs/heads/master.zip; \
		unzip master.zip; \
		mv ffmpeg-rockchip-master $(FFMPEG_DIR); \
	fi
	if [ ! -e $(ROOT_PATH)/rockchip/ffmpeg_rockchip ]; then \
		export LD_LIBRARY_PATH=$(ROOT_PATH)/opencv/x264_install_$(X264_VER)/lib/:$(ROOT_PATH)/rockchip/linux-rga/lib/:$(ROOT_PATH)/rockchip/mpp/lib/:$(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER)/lib; \
		export PKG_CONFIG_LIBDIR=$(PKG_CONFIG_LIBDIR):$(ROOT_PATH)/opencv/x264_install_$(X264_VER)/lib/:$(ROOT_PATH)/rockchip/lib/:$(ROOT_PATH)/rockchip/linux-rga/lib/:$(ROOT_PATH)/rockchip/mpp/lib/:$(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER)/lib; \
		export PKG_CONFIG_PATH=$(PKG_CONFIG_PATH):$(ROOT_PATH)/opencv/x264_install_$(X264_VER)/lib/pkgconfig:$(ROOT_PATH)/rockchip/lib/pkgconfig:$(ROOT_PATH)/rockchip/mpp/lib/pkgconfig:$(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER)/lib/pkgconfig; \
		cd $(FFMPEG_DIR)&&./configure --prefix=$(ROOT_PATH)/rockchip/ffmpeg_rockchip --enable-cross-compile --arch=$(ARCH) \
		--disable-stripping --target-os=linux --cc=$(CC) \
		--pkg-config-flags="--static" \
		--disable-asm --disable-x86asm --disable-inline-asm \
		--enable-decoder=aac --enable-decoder=aac_fixed --enable-decoder=aac_latm --enable-encoder=aac \
		--extra-libs="-lpthread -lm" \
		--enable-shared --disable-static \
		--enable-version3 --enable-libdrm --enable-rkmpp   \
		--enable-gpl --enable-pic --enable-nonfree --enable-pthreads; \
		make -C $(FFMPEG_DIR); \
		make -C $(FFMPEG_DIR) install; \
	fi
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/lib/* $(ROOT_PATH)/rockchip/lib -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/bin/* $(ROOT_PATH)/rockchip/bin -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/include/*  $(ROOT_PATH)/rockchip/include -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/bin/* $(ROOT_PATH)/opencv/bin -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/include/*  $(ROOT_PATH)/opencv/include -rf


ffmpeg_rockchip_install:
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/lib/* $(CURDIR)/../Libs/$(SOC)/ffmpeg -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/bin/* $(CURDIR)/../Libs/$(SOC)/ffmpeg/bin -rf
	cp $(ROOT_PATH)/rockchip/ffmpeg_rockchip/include/* $(CURDIR)/../Libs/$(SOC)/ffmpeg/include -rf

ffmpeg_rockchip_clean:
	rm $(ROOT_PATH)/rockchip/ffmpeg_rockchip -rf
	rm $(ROOT_PATH)/ffmpeg-rockchip-master -rf

#opencv

opencvenv:
	if [ ! -e $(ROOT_PATH)/opencv ]; then \
		mkdir -p $(ROOT_PATH)/opencv; \
		mkdir -p $(ROOT_PATH)/opencv/bin; \
		mkdir -p $(ROOT_PATH)/opencv/lib; \
		mkdir -p $(ROOT_PATH)/opencv/include; \
	fi
#https://www.alsa-project.org/files/pub/lib/alsa-lib-1.2.9.tar.bz2
alsa_lib:opencvenv
	if [ ! -e $(ROOT_PATH)/opencv/alsa_lib_install_$(ALSA_VER) ]; then \
		tar -xvjf $(ROOT_PATH)/alsa-lib-$(ALSA_VER).tar.bz2; \
		cd $(ROOT_PATH)/alsa-lib-$(ALSA_VER)/&&./configure --prefix=$(ROOT_PATH)/opencv/alsa_lib_install_$(ALSA_VER) --host=$(CROSS_COMPILE_HOST) --disable-static --disable-python --without-debug; \
		make -C $(ROOT_PATH)/alsa-lib-$(ALSA_VER)/; \
		make -C $(ROOT_PATH)/alsa-lib-$(ALSA_VER)/ install; \
	fi
	cp $(ROOT_PATH)/opencv/alsa_lib_install_$(ALSA_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/alsa_lib_install_$(ALSA_VER)/include/* $(ROOT_PATH)/opencv/include -rf

fdkaac:opencvenv
	if [ ! -e $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER) ]; then \
		if [ -e $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER).zip ]; then unzip $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER).zip; else echo "please download https://github.com/mstorsjo/fdk-aac.git";fi; \
	fi
	if [ ! -e $(ROOT_PATH)/opencv/fdkaac_install_$(FDKAAC_VER) ]; then \
		mkdir -p $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER)/build; \
		cd $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER)/build&&cmake -DCMAKE_INSTALL_PREFIX=$(ROOT_PATH)/opencv/fdkaac_install_$(FDKAAC_VER) -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) ..;\
		cd $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER)/build&&make; \
		cd $(ROOT_PATH)/fdk-aac-$(FDKAAC_VER)/build&&make install; \
	fi

fdkaac_install:
	mkdir -p $(CURDIR)/../Libs/$(SOC)/fakacc
	cp $(ROOT_PATH)/opencv/fdkaac_install_$(FDKAAC_VER)/lib  $(CURDIR)/../Libs/$(SOC)/fdkacc/ -rf
	cp $(ROOT_PATH)/opencv/fdkaac_install_$(FDKAAC_VER)/include $(CURDIR)/../Libs/$(SOC)/fdkacc/ -rf
	
zlib:opencvenv
	if [ ! -e $(ROOT_PATH)/opencv/zlib_install_$(ZLIB_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/zlib-$(ZLIB_VER).tar.gz; \
		cd $(ROOT_PATH)/zlib-$(ZLIB_VER)/&&./configure --prefix=$(ROOT_PATH)/opencv/zlib_install_$(ZLIB_VER) --shared; \
		make -C $(ROOT_PATH)/zlib-$(ZLIB_VER)/; \
		make -C $(ROOT_PATH)/zlib-$(ZLIB_VER)/ install; \
	fi
	cp $(ROOT_PATH)/opencv/zlib_install_$(ZLIB_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/zlib_install_$(ZLIB_VER)/include/* $(ROOT_PATH)/opencv/include -rf

jpegsrc:opencvenv
	if [ ! -e $(ROOT_PATH)/opencv/jpegsrc_install_$(JPEGSRC_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/jpegsrc.$(JPEGSRC_VER).tar.gz; \
		cd $(ROOT_PATH)/$(JPEGSRC_DIR)/&&./configure --prefix=$(ROOT_PATH)/opencv/jpegsrc_install_$(JPEGSRC_VER) --enable-shared --enable-static --host=$(CROSS_COMPILE_HOST); \
		make -C $(ROOT_PATH)/$(JPEGSRC_DIR)/; \
		make -C $(ROOT_PATH)/$(JPEGSRC_DIR)/ install; \
	fi
	cp $(ROOT_PATH)/opencv/jpegsrc_install_$(JPEGSRC_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/jpegsrc_install_$(JPEGSRC_VER)/include/* $(ROOT_PATH)/opencv/include -rf
	cp $(ROOT_PATH)/opencv/jpegsrc_install_$(JPEGSRC_VER)/bin/* $(ROOT_PATH)/opencv/bin -rf

libpng:zlib
	if [ ! -e $(ROOT_PATH)/opencv/libpng_install_$(LIBPNG_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/libpng-$(LIBPNG_VER).tar.gz; \
		cd $(ROOT_PATH)/libpng-$(LIBPNG_VER)/&&./configure -host=$(CROSS_COMPILE_HOST) -prefix=$(ROOT_PATH)/opencv/libpng_install_$(LIBPNG_VER) --enable-shared --enable-static LIBS=-L$(ROOT_PATH)/opencv/zlib_install_$(ZLIB_VER)/lib CPPFLAGS=-I$(ROOT_PATH)/opencv/zlib_install_$(ZLIB_VER)/include; \
		make -C $(ROOT_PATH)/libpng-$(LIBPNG_VER); \
		make -C $(ROOT_PATH)/libpng-$(LIBPNG_VER) install; \
	fi
	cp $(ROOT_PATH)/opencv/libpng_install_$(LIBPNG_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/libpng_install_$(LIBPNG_VER)/include/* $(ROOT_PATH)/opencv/include -rf
	cp $(ROOT_PATH)/opencv/libpng_install_$(LIBPNG_VER)/bin/* $(ROOT_PATH)/opencv/bin -rf

yasm:opencvenv
	if [ ! -e $(ROOT_PATH)/opencv/yasm_install_$(YASM_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/yasm-$(YASM_VER).tar.gz; \
		cd $(ROOT_PATH)/yasm-$(YASM_VER)/&&./configure --prefix=$(ROOT_PATH)/opencv/yasm_install_$(YASM_VER) --host=$(CROSS_COMPILE_HOST); \
		make -C $(ROOT_PATH)/yasm-$(YASM_VER)/; \
		make -C $(ROOT_PATH)/yasm-$(YASM_VER)/ install; \
	fi
	cp $(ROOT_PATH)/opencv/yasm_install_$(YASM_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/yasm_install_$(YASM_VER)/include/* $(ROOT_PATH)/opencv/include -rf
	cp $(ROOT_PATH)/opencv/yasm_install_$(YASM_VER)/bin/* $(ROOT_PATH)/opencv/bin -rf

x264:opencvenv
	if [ ! -e $(ROOT_PATH)/x264-$(X264_VER) ]; then \
		wget https://code.videolan.org/videolan/x264/-/archive/master/x264-master.tar.bz2; \
		tar -xvjf $(ROOT_PATH)/x264-$(X264_VER).tar.bz2; \
	fi
	if [ ! -e $(ROOT_PATH)/opencv/x264_install_$(X264_VER) ]; then \
		tar -xvjf $(ROOT_PATH)/x264-$(X264_VER).tar.bz2; \
		cd $(ROOT_PATH)/x264-$(X264_VER)/&&./configure --prefix=$(ROOT_PATH)/opencv/x264_install_$(X264_VER)  --disable-asm --enable-shared --enable-static --host=$(CROSS_COMPILE_HOST) --enable-pic; \
		make -C $(ROOT_PATH)/x264-$(X264_VER)/; \
		make -C $(ROOT_PATH)/x264-$(X264_VER)/ install; \
	fi
	cp $(ROOT_PATH)/opencv/x264_install_$(X264_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/x264_install_$(X264_VER)/include/* $(ROOT_PATH)/opencv/include -rf
	cp $(ROOT_PATH)/opencv/x264_install_$(X264_VER)/bin/* $(ROOT_PATH)/opencv/bin -rf

x264_clean:
	rm $(ROOT_PATH)/opencv/x264_install_$(DRM_VER) -rf
	rm $(ROOT_PATH)/x264-$(X264_VER) -rf


xvidcore:opencvenv
	if [ ! -e $(ROOT_PATH)/opencv/xvidcore_install_$(XVID_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/xvidcore-$(XVID_VER).tar.gz; \
		cd $(ROOT_PATH)/xvidcore/build/generic &&./configure --prefix=$(ROOT_PATH)/opencv/xvidcore_install_$(XVID_VER) --disable-assembly --host=$(CROSS_COMPILE_HOST); \
		make -C $(ROOT_PATH)/xvidcore/build/generic; \
		make -C $(ROOT_PATH)/xvidcore/build/generic install; \
	fi
	cp $(ROOT_PATH)/opencv/xvidcore_install_$(XVID_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/xvidcore_install_$(XVID_VER)/include/* $(ROOT_PATH)/opencv/include -rf


libdrm:opencvenv
	if [ ! -e libdrm-$(DRM_VER) ]; then \
		wget https://dri.freedesktop.org/libdrm/libdrm-2.4.124.tar.xz; \
	fi
	echo "[binaries]"              >  $(ROOT_PATH)/cross_file.txt
	echo "c = '$(CC)'"             >> $(ROOT_PATH)/cross_file.txt
	echo "cpp = '$(CXX)'"          >> $(ROOT_PATH)/cross_file.txt
	echo "ar = '$(AR)'"            >> $(ROOT_PATH)/cross_file.txt
	echo "strip = '$(STRIP)'"      >> $(ROOT_PATH)/cross_file.txt
	echo "  "                      >> $(ROOT_PATH)/cross_file.txt
	echo "[host_machine]"          >> $(ROOT_PATH)/cross_file.txt
	echo "system = 'linux'"        >> $(ROOT_PATH)/cross_file.txt
	echo "cpu_family = 'aarch64'"  >> $(ROOT_PATH)/cross_file.txt
	echo "cpu = 'aarch64'"         >> $(ROOT_PATH)/cross_file.txt
	echo "endian = 'little'"       >> $(ROOT_PATH)/cross_file.txt
	if [ ! -e $(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER) ]; then \
		tar -xvf $(ROOT_PATH)/libdrm-$(DRM_VER).tar.xz; \
		mv $(ROOT_PATH)/cross_file.txt $(ROOT_PATH)/libdrm-$(DRM_VER)/; \
		cd $(ROOT_PATH)/libdrm-$(DRM_VER)/ && meson setup --prefix=$(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER) --cross-file $(ROOT_PATH)/libdrm-$(DRM_VER)/cross_file.txt $(ROOT_PATH)/libdrm-$(DRM_VER)/build; \
		cd $(ROOT_PATH)/libdrm-$(DRM_VER)/build && ninja && ninja install; \
	fi
	cp $(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER)/include/* $(ROOT_PATH)/opencv/include -rf

libdrm_clean:
	rm $(ROOT_PATH)/opencv/libdrm_install_$(DRM_VER) -rf
	rm $(ROOT_PATH)/libdrm-$(DRM_VER) -rf


ffmpeg:libpng jpegsrc yasm x264 xvidcore
	if [ ! -e $(ROOT_PATH)/opencv/ffmpeg_install_$(FFMPEG_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/ffmpeg-$(FFMPEG_VER).tar.gz; \
		export LD_LIBRARY_PATH=$(ROOT_PATH)/opencv/lib; \
		export PATH=$(PATH):$(ROOT_PATH)/opencv/bin; \
		cd $(ROOT_PATH)/ffmpeg-$(FFMPEG_VER) && ./configure --prefix=$(ROOT_PATH)/opencv/ffmpeg_install_$(FFMPEG_VER) --enable-shared --disable-static --enable-gpl --enable-cross-compile  --arch=$(ARCH) --disable-stripping --target-os=linux --enable-libx264 --enable-libxvid --cc=$(CC) --enable-swscale --extra-ldflags=-L$(ROOT_PATH)/opencv/lib --extra-cflags=-I$(ROOT_PATH)/opencv/include; \
		make -C $(ROOT_PATH)/ffmpeg-$(FFMPEG_VER); \
		make -C $(ROOT_PATH)/ffmpeg-$(FFMPEG_VER) install; \
	fi
	cp $(ROOT_PATH)/opencv/ffmpeg_install_$(FFMPEG_VER)/lib/* $(ROOT_PATH)/opencv/lib -rf
	cp $(ROOT_PATH)/opencv/ffmpeg_install_$(FFMPEG_VER)/include/* $(ROOT_PATH)/opencv/include -rf
	cp $(ROOT_PATH)/opencv/ffmpeg_install_$(FFMPEG_VER)/bin/* $(ROOT_PATH)/opencv/bin -rf

ffmpeg_clean:
	rm $(ROOT_PATH)/ffmpeg-$(FFMPEG_VER) -rf
	rm $(ROOT_PATH)/xvidcore -rf
	rm $(ROOT_PATH)/x264-$(X264_VER) -rf
	rm $(ROOT_PATH)/yasm-$(YASM_VER) -rf
	rm $(ROOT_PATH)/libpng-$(LIBPNG_VER) -rf
	rm $(ROOT_PATH)/$(JPEGSRC_DIR) -rf
	rm $(ROOT_PATH)/zlib-$(ZLIB_VER) -rf

opencv:ffmpeg_rockchip
	if [ ! -e opencv-$(OPENCV_VER) ]; then \
		wget https://github.com/opencv/opencv/archive/refs/tags/$(OPENCV_VER).zip; \
		unzip $(OPENCV_VER).zip; \
	fi
	if [ ! -e $(ROOT_PATH)/opencv-$(OPENCV_VER)/build/ ]; then \
		export PKG_CONFIG_PATH=$(ROOT_PATH)/opencv/lib/pkgconfig; \
		export LD_LIBRARY_PATH=$(TARGET_COMPILE_LIBCDIR):$(ROOT_PATH)/opencv/lib; \
		export PKG_CONFIG_LIBDIR=$(PKG_CONFIG_LIBDIR):$(ROOT_PATH)/opencv/lib; \
		touch $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake;\
		echo "set(CMAKE_SYSTEM_PROCESSOR aarch64)" > $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "set(GCC_COMPILER_VERSION \"\" CACHE STRING \"GCC Compiler version\")" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "set(GNU_MACHINE \"aarch64-linux-gnu\" CACHE STRING \"GNU compiler triple\")" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "set( OPENCV_ENABLE_PKG_CONFIG ON)" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "set( CMAKE_C_COMPILER $(CC))" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "set( CMAKE_CXX_COMPILER $(CXX))" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "set( CMAKE_C_FLAGS \"-Wl,-rpath-link=$(ROOT_PATH)/opencv/lib\")" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		echo "include(\"\$$$\{CMAKE_CURRENT_LIST_DIR$\}/arm.toolchain.cmake\")" >> $(ROOT_PATH)/opencv-$(OPENCV_VER)/platforms/linux/$(CROSS_COMPILE_HOST).cmake; \
		mkdir -p $(ROOT_PATH)/opencv-$(OPENCV_VER)/build; \
		cd $(ROOT_PATH)/opencv-$(OPENCV_VER)/build&&cmake -D CMAKE_TOOLCHAIN_FILE=../platforms/linux/$(CROSS_COMPILE_HOST).cmake ../; \
		make -C $(ROOT_PATH)/opencv-$(OPENCV_VER)/build; \
		make -C $(ROOT_PATH)/opencv-$(OPENCV_VER)/build install; \
	fi
	mkdir -p $(ROOT_PATH)/opencv/install
	mkdir -p $(ROOT_PATH)/opencv/install/lib
	mkdir -p $(ROOT_PATH)/opencv/install/include
	cp $(ROOT_PATH)/opencv-$(OPENCV_VER)/build/install/lib/*.so* $(ROOT_PATH)/opencv/install/lib/ -rf
	cp $(ROOT_PATH)/opencv-$(OPENCV_VER)/build/install/include/opencv4/* $(ROOT_PATH)/opencv/install/include/ -rf
	cp $(ROOT_PATH)/opencv-$(OPENCV_VER)/build/install/bin/* $(ROOT_PATH)/opencv/bin/ -rf

opencv_clean:ffmpeg_clean
	rm $(ROOT_PATH)/opencv-$(OPENCV_VER) -rf

opencv_install:
	mkdir -p $(CURDIR)/../Libs/opencv/
	mkdir -p $(CURDIR)/../Libs/opencv/include
	cp $(ROOT_PATH)/opencv/install/lib/*.so* $(CURDIR)/../Libs/opencv/ -rf
	cp $(ROOT_PATH)/opencv/install/include $(CURDIR)/../Libs/opencv/include -rf


# wifi
wifienv:
	if [ ! -e $(ROOT_PATH)/wifi ]; then \
		mkdir -p $(ROOT_PATH)/wifi; \
		mkdir -p $(ROOT_PATH)/wifi/output/lib; \
		mkdir -p $(ROOT_PATH)/wifi/output/bin; \
	fi
# openssl website : https://github.com/openssl/openssl/releases
openssl:wifienv
	if [ ! -e $(ROOT_PATH)/wifi/openssl_install_$(OPENSSL_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/openssl-$(OPENSSL_VER).tar.gz; \
		cd $(ROOT_PATH)/openssl-$(OPENSSL_VER)/&&./config no-asm shared no-async --prefix=$(ROOT_PATH)/wifi/openssl_install_$(OPENSSL_VER); \
		sed -i "s/PLATFORM=linux-x86_64/PLATFORM=arm/" $(ROOT_PATH)/openssl-$(OPENSSL_VER)/Makefile; \
		sed -i "s/CNF_CFLAGS=-pthread -m64/CNF_CFLAGS=-pthread/" $(ROOT_PATH)/openssl-$(OPENSSL_VER)/Makefile; \
		sed -i "s/CNF_CXXFLAGS=-std=c++11 -pthread -m64/CNF_CXXFLAGS=-std=c++11 -pthread/" $(ROOT_PATH)/openssl-$(OPENSSL_VER)/Makefile; \
		make -C $(ROOT_PATH)/openssl-$(OPENSSL_VER)/; \
		make -C $(ROOT_PATH)/openssl-$(OPENSSL_VER)/ install; \
	fi
	
# libnl website :http://www.infradead.org/~tgr/libnl/
libnl:wifienv
	if [ ! -e $(ROOT_PATH)/wifi/libnl_install_$(LIBNL_VER) ]; then \
		tar -zxvf $(ROOT_PATH)/libnl-$(LIBNL_VER).tar.gz; \
		cd $(ROOT_PATH)/libnl-$(LIBNL_VER)/&&./configure  --host=$(CROSS_COMPILE_HOST) --prefix=$(ROOT_PATH)/wifi/libnl_install_$(LIBNL_VER) --sysconfdir=$(ROOT_PATH)/wifi/libnl_etc --enable-shared --enable-static; \
		make -C $(ROOT_PATH)/libnl-$(LIBNL_VER); \
		make -C $(ROOT_PATH)/libnl-$(LIBNL_VER) install; \
	fi

# wpa_supplicant website :http://w1.fi/wpa_supplicant/
wpa_supplicant:openssl libnl
	tar -zxvf $(ROOT_PATH)/wpa_supplicant-$(WPA_VER).tar.gz
	cp $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/defconfig $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	sed -i "s/CONFIG_CTRL_IFACE_DBUS_NEW=y/#CONFIG_CTRL_IFACE_DBUS_NEW=y/" $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	sed -i "s/CONFIG_CTRL_IFACE_DBUS_INTRO=y/#CONFIG_CTRL_IFACE_DBUS_INTRO=y/" $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	echo "CFLAGS+=-I$(ROOT_PATH)/wifi/openssl_install_$(OPENSSL_VER)/include" >> $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	echo "LIBS+=-L$(ROOT_PATH)/wifi/openssl_install_$(OPENSSL_VER)/lib64" >> $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	echo "CFLAGS+=-I$(ROOT_PATH)/wifi/libnl_install_$(LIBNL_VER)/include/libnl3" >> $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	echo "LIBS+=-L$(ROOT_PATH)/wifi/libnl_install_$(LIBNL_VER)/lib" >> $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	echo "CC=$(CC)" >> $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/.config
	make -C $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/

wpa_supplicant_install:
	$(INSTALL) $(ROOT_PATH)/wifi/openssl_install_$(OPENSSL_VER)/lib64/*.so*                         $(ROOT_PATH)/wifi/output/lib
	$(INSTALL) $(ROOT_PATH)/wifi/libnl_install_$(LIBNL_VER)/lib/*.so*                               $(ROOT_PATH)/wifi/output/lib
	$(INSTALL) $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/wpa_cli                        $(ROOT_PATH)/wifi/output/bin
	$(INSTALL) $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/wpa_passphrase                 $(ROOT_PATH)/wifi/output/bin
	$(INSTALL) $(ROOT_PATH)/wpa_supplicant-$(WPA_VER)/wpa_supplicant/wpa_supplicant                 $(ROOT_PATH)/wifi/output/bin
	$(INSTALL) $(ROOT_PATH)/wpa_supplicant.conf                                                     $(ROOT_PATH)/wifi/output/


wpa_supplicant_clean:
	rm $(ROOT_PATH)/wifi -rf
	rm $(ROOT_PATH)/wpa_supplicant-$(WPA_VER) -rf
	rm $(ROOT_PATH)/libnl-$(LIBNL_VER) -rf
	rm $(ROOT_PATH)/openssl-$(OPENSSL_VER) -rf

mp4v2:opencvenv
	if [ ! -e mp4v2-$(MP4V2_VER) ]; then \
		if [ -e $(ROOT_PATH)/mp4v2-$(MP4V2_VER).zip ]; then unzip $(ROOT_PATH)/mp4v2-$(MP4V2_VER).zip; else echo "please download https://github.com/TechSmith/mp4v2";fi; \
	fi
	mkdir -p $(ROOT_PATH)/mp4v2-$(MP4V2_VER)/build
	cd $(ROOT_PATH)/mp4v2-$(MP4V2_VER)/build&&cmake -DCMAKE_INSTALL_PREFIX=$(ROOT_PATH)/opencv/mp4v2_install_$(MP4V2_VER) -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) ..
	cd $(ROOT_PATH)/mp4v2-$(MP4V2_VER)/build&&make
	mkdir -p $(ROOT_PATH)/opencv/mp4v2_install/lib
	mkdir -p $(ROOT_PATH)/opencv/mp4v2_install/include 
	cp $(ROOT_PATH)/mp4v2-$(MP4V2_VER)/build/libmp4v2.so $(ROOT_PATH)/opencv/mp4v2_install/lib/ -rf
	cp $(ROOT_PATH)/mp4v2-$(MP4V2_VER)/include/* $(ROOT_PATH)/opencv/mp4v2_install/include/ -rf


mp4v2_install:
	mkdir -p $(CURDIR)/../Libs/$(SOC)/mp4v2
	cp $(ROOT_PATH)/opencv/mp4v2_install/lib  $(CURDIR)/../Libs/$(SOC)/mp4v2/ -rf
	cp $(ROOT_PATH)/opencv/mp4v2_install/include $(CURDIR)/../Libs/$(SOC)/mp4v2/ -rf