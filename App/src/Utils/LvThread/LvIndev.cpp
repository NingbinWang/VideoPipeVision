#include "lvgl/lvgl.h"
#include "TouchScreen.h"
#include "Logger.h"

static TouchScreen ts;

// LVGL输入设备读取回调函数
void Touchscreen_read_cb(lv_indev_t * indev, lv_indev_data_t * data) {
        TouchPoint tp = ts.getLastTouchPoint();
        
        if (ts.isTouchActive()) {
            data->point.x = tp.x;
            data->point.y = tp.y;
            data->state = LV_INDEV_STATE_PRESSED;
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
}
int LvLinuxIndevInit(void)
{
    lv_indev_t* indev_drv;
     // 枚举所有输入设备
    auto devices = InputDevice::enumerateDevices();
    for (size_t i = 0; i < devices.size(); ++i) {
        LOG_INFO("input device:%s path:%s\n",devices[i].name.c_str(), devices[i].path.c_str());
    }
    
    // 尝试找到触摸屏设备
    std::string touch_device_path = "";
    for (const auto& dev : devices) {
        // 根据设备名称判断是否为触摸屏
        std::string name_lower = dev.name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
        if (name_lower.find("goodix-ts") != std::string::npos) {
            touch_device_path = dev.path;
            break;
        }
    }
    
    if (touch_device_path.empty()) {
            LOG_ERROR("NO FOUND INPUT DEVICE!!!!\n");
            return -1;

    }
    LOG_INFO("INPUT Path:%s\n",touch_device_path.c_str());
    
    if (!ts.openDevice(touch_device_path)) {
        LOG_ERROR("can't open device: %s \n", touch_device_path.c_str());
        return -1;
    }
    
    ts.setTouchCallback([](const TouchPoint& tp) {
        LOG_INFO("setTouchCallback: [ %d, %d] ID:%d\n",tp.x,tp.y,tp.tracking_id);
    });
    
    ts.setMultiTouchCallback([](const TouchPoint& tp) {
        LOG_INFO("setMultiTouchCallback: [ %d, %d] ID:%d vaild:%d\n",tp.x,tp.y,tp.tracking_id,tp.valid);
    });

    ts.startListening();
    
	indev_drv = lv_indev_create();
	lv_indev_set_type(indev_drv,LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(indev_drv,Touchscreen_read_cb);
    return 0;
}


