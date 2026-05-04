#!/bin/sh
CURRENT_DIR=$(cd "$(dirname "$(realpath "$0")")/.."; pwd)
# opencv4.10
ln -s $CURRENT_DIR/lib/libopencv_calib3d.so                /lib/libopencv_calib3d.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_calib3d.so                /lib/libopencv_calib3d.so.410
ln -s $CURRENT_DIR/lib/libopencv_core.so                   /lib/libopencv_core.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_core.so                   /lib/libopencv_core.so.410
ln -s $CURRENT_DIR/lib/libopencv_dnn.so                    /lib/libopencv_dnn.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_dnn.so                    /lib/libopencv_dnn.so.410
ln -s $CURRENT_DIR/lib/libopencv_features2d.so             /lib/libopencv_features2d.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_features2d.so             /lib/libopencv_features2d.so.410
ln -s $CURRENT_DIR/lib/libopencv_flann.so                  /lib/libopencv_flann.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_flann.so                  /lib/libopencv_flann.so.410
ln -s $CURRENT_DIR/lib/libopencv_highgui.so                /lib/libopencv_highgui.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_highgui.so                /lib/libopencv_highgui.so.410
ln -s $CURRENT_DIR/lib/libopencv_imgcodecs.so              /lib/libopencv_imgcodecs.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_imgcodecs.so              /lib/libopencv_imgcodecs.so.410
ln -s $CURRENT_DIR/lib/libopencv_imgproc.so                /lib/libopencv_imgproc.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_imgproc.so                /lib/libopencv_imgproc.so.410
ln -s $CURRENT_DIR/lib/libopencv_ml.so                     /lib/libopencv_ml.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_ml.so                     /lib/libopencv_ml.so.410
ln -s $CURRENT_DIR/lib/libopencv_objdetect.so              /lib/libopencv_objdetect.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_objdetect.so              /lib/libopencv_objdetect.so.410
ln -s $CURRENT_DIR/lib/libopencv_photo.so                  /lib/libopencv_photo.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_photo.so                  /lib/libopencv_photo.so.410
ln -s $CURRENT_DIR/lib/libopencv_stitching.so              /lib/libopencv_stitching.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_stitching.so              /lib/libopencv_stitching.so.410
ln -s $CURRENT_DIR/lib/libopencv_video.so                  /lib/libopencv_video.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_video.so                  /lib/libopencv_video.so.410
ln -s $CURRENT_DIR/lib/libopencv_videoio.so                /lib/libopencv_videoio.so.4.10.0
ln -s $CURRENT_DIR/lib/libopencv_videoio.so                /lib/libopencv_videoio.so.410
# ffmpeg
ln -s $CURRENT_DIR/lib/libavcodec.so                       /lib/libavcodec.so.60
ln -s $CURRENT_DIR/lib/libavcodec.so                       /lib/libavcodec.so.60.31.102
ln -s $CURRENT_DIR/lib/libavdevice.so                      /lib/libavdevice.so.60
ln -s $CURRENT_DIR/lib/libavdevice.so                      /lib/libavdevice.so.60.3.100
ln -s $CURRENT_DIR/lib/libavfilter.so                      /lib/libavfilter.so.9
ln -s $CURRENT_DIR/lib/libavfilter.so                      /lib/libavfilter.so.9.12.100
ln -s $CURRENT_DIR/lib/libavformat.so                      /lib/libavformat.so.60
ln -s $CURRENT_DIR/lib/libavformat.so                      /lib/libavformat.so.60.16.100
ln -s $CURRENT_DIR/lib/libavutil.so                        /lib/libavutil.so.58
ln -s $CURRENT_DIR/lib/libavutil.so                        /lib/libavutil.so.60
ln -s $CURRENT_DIR/lib/libavutil.so                        /lib/libavutil.so.60.3.100
ln -s $CURRENT_DIR/lib/libpostproc.so                      /lib/libpostproc.so.57
ln -s $CURRENT_DIR/lib/libpostproc.so                      /lib/libpostproc.so.57.3.100
ln -s $CURRENT_DIR/lib/libswresample.so                    /lib/libswresample.so.4
ln -s $CURRENT_DIR/lib/libswresample.so                    /lib/libswresample.so.4.12.100
ln -s $CURRENT_DIR/lib/libswscale.so                       /lib/libswscale.so.7
ln -s $CURRENT_DIR/lib/libswscale.so                       /lib/libswscale.so.7.5.100
#libx264
ln -s $CURRENT_DIR/lib/libx264.so                          /lib/libx264.so.165
#mpp
ln -s $CURRENT_DIR/lib/librockchip_mpp.so                  /lib/librockchip_mpp.so.0
ln -s $CURRENT_DIR/lib/librockchip_mpp.so                  /lib/librockchip_mpp.so.1
ln -s $CURRENT_DIR/lib/librockchip_vpu.so                  /lib/librockchip_vpu.so.0
ln -s $CURRENT_DIR/lib/librockchip_vpu.so                  /lib/librockchip_vpu.so.1
#libdrm
ln -s $CURRENT_DIR/lib/libdrm.so                           /lib/libdrm.so.2
ln -s $CURRENT_DIR/lib/libdrm.so                           /lib/libdrm.so.2.124.0
ln -s $CURRENT_DIR/lib/libdrm_etnaviv.so                   /lib/libdrm_etnaviv.so.1
ln -s $CURRENT_DIR/lib/libdrm_etnaviv.so                   /lib/libdrm_etnaviv.so.1.124.0
ln -s $CURRENT_DIR/lib/libdrm_freedreno.so                 /lib/libdrm_freedreno.so.1
ln -s $CURRENT_DIR/lib/libdrm_freedreno.so                 /lib/libdrm_freedreno.so.1.124.0
ln -s $CURRENT_DIR/lib/libdrm_nouveau.so                   /lib/libdrm_nouveau.so.2
ln -s $CURRENT_DIR/lib/libdrm_nouveau.so                   /lib/libdrm_nouveau.so.2.124.0
ln -s $CURRENT_DIR/lib/libdrm_radeon.so                    /lib/libdrm_radeon.so.1
ln -s $CURRENT_DIR/lib/libdrm_radeon.so                    /lib/libdrm_radeon.so.1.124.0
ln -s $CURRENT_DIR/lib/libdrm_amdgpu.so                    /lib/libdrm_amdgpu.so.1
ln -s $CURRENT_DIR/lib/libdrm_amdgpu.so                    /lib/libdrm_amdgpu.so.1.124.0
#fdk-acc
ln -s $CURRENT_DIR/lib/libfdk-aac.so                       /lib/libfdk-aac.so.2
