#ifndef _AUTO_CONF_H_
#define _AUTO_CONF_H_

//wifi
#define WIFI_MODE 0
#define WIFI_SSID "alex"
#define WIFI_AP_DEFAULTPASSWORD "alex"
//AI
#define LABEL_NALE_TXT_PATH "/home/cat/coco_80_labels_list.txt"
#define MODEL_PATH "/home/cat/yolov5s-640-640.rknn"
//audio
#define AUDIODEVNAME   "default"

//主子码流节点名
#define MAINDEVNAME   "/dev/video0"
#define MAINFORMAT   "NV12"
#define SUBDEVNAME   "/dev/video1"
//RTSP是使用的网卡名
#define RTSPNETDEVNAME "eth0"
//lvgl的控制信息
#define STARTUP_LABLE "LVGL"
#define CONFIG_STATUS_BAR_HEIGHT 30
#define CONFIG_STATUSBAR_SHOW_STATELLITE 0
#define CONFIG_STATUSBAR_SHOW_SDCARD 0
#define CONFIG_STATUSBAR_SHOW_BLUETOOTH 0
#define CONFIG_STATUSBAR_SHOW_TIME 1
#define CONFIG_STATUSBAR_SHOWBATTERY 0

#endif