#ifndef TOUCH_SCREEN_H
#define TOUCH_SCREEN_H

#include "InputDevice.h"
#include <iostream>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <dirent.h>
#include <cstring>
#include <algorithm>

struct TouchPoint {
    int x = 0;
    int y = 0;
    int pressure = 0;
    int tracking_id = -1;
    bool valid = false;
};

class TouchScreen : public InputDevice {
public:
    using TouchCallback = std::function<void(const TouchPoint&)>;
    
    TouchScreen();

	// 获取最新的触摸点数据
TouchPoint getLastTouchPoint() const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return current_point_;
}
    
// 获取多点触控数据
std::map<int, TouchPoint> getMultiTouchPoints() const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return multi_touch_points_;
}
    
bool isTouchActive() const {
        std::lock_guard<std::mutex> lock(data_mutex_);
        return current_point_.valid;
}
void setTouchCallback(TouchCallback callback) { touch_callback_ = callback; }
void setMultiTouchCallback(TouchCallback callback) { multitouch_callback_ = callback; }

    
    // 重写事件处理
    void onAbsEvent(const input_event& ev) override;
    void onSynEvent(const input_event& ev) override;

private:
    mutable std::mutex data_mutex_;
    TouchCallback touch_callback_;
    TouchCallback multitouch_callback_;
    TouchPoint current_point_;
    std::map<int, TouchPoint> multi_touch_points_;
	int current_slot_ = 0;
};

#endif // TOUCH_SCREEN_H
