#include "TouchScreen.h"
#include <iostream>
#include "Logger.h"

TouchScreen::TouchScreen() {}

void TouchScreen::onAbsEvent(const input_event& ev) {
	std::lock_guard<std::mutex> lock(data_mutex_);
    switch (ev.code) {
        case ABS_X:
            current_point_.x = ev.value;
            break;
        case ABS_Y:
            current_point_.y = ev.value;
            break;
        case ABS_PRESSURE:
            current_point_.pressure = ev.value;
            break;
        case ABS_MT_SLOT:
            // 多点触控槽位
            current_slot_ = ev.value;
            break;
        case ABS_MT_TRACKING_ID:
		    if (current_slot_ >= 0 && current_slot_ < 10) {
                    multi_touch_points_[current_slot_].tracking_id = ev.value;
                    multi_touch_points_[current_slot_].valid = (ev.value != -1);
                    
                    // 如果是新的触摸点，也更新到current_point_
                    if (ev.value != -1) {
                        current_point_.x = multi_touch_points_[current_slot_].x;
                        current_point_.y = multi_touch_points_[current_slot_].y;
                        current_point_.pressure = multi_touch_points_[current_slot_].pressure;
                        current_point_.valid = true;
                    } else {
                        // 如果是结束触摸，检查是否还有其他触摸点
                        bool has_active_touch = false;
                        for (const auto& pair : multi_touch_points_) {
                            if (pair.second.valid) {
                                has_active_touch = true;
                                break;
                            }
                        }
                        if (!has_active_touch) {
                            current_point_.valid = false;
                        }
                    }
                }
            break;
        case ABS_MT_POSITION_X:
			if (current_slot_ >= 0 && current_slot_ < 10) {
                    multi_touch_points_[current_slot_].x = ev.value;
                    // 如果当前没有活动触摸，将第一个触摸点设为活动
                    if (!current_point_.valid) {
                        current_point_.x = ev.value;
                        current_point_.valid = true;
                    }
            }
            break;
        case ABS_MT_POSITION_Y:
            if (current_slot_ >= 0 && current_slot_ < 10) {
                    multi_touch_points_[current_slot_].y = ev.value;
                    // 如果当前没有活动触摸，将第一个触摸点设为活动
                    if (!current_point_.valid) {
                        current_point_.y = ev.value;
                        current_point_.valid = true;
                    }
                }
            break;
        default:
            break;
    }
}

void TouchScreen::onSynEvent(const input_event& ev) {
    if (ev.code == SYN_REPORT) {
            std::lock_guard<std::mutex> lock(data_mutex_);
            
            // 如果有单点触摸回调，处理当前点
            if (current_point_.valid && touch_callback_) {
                touch_callback_(current_point_);
            }
            
            // 如果有多点触摸回调，处理所有活动的触摸点
            if (multitouch_callback_) {
                for (auto& pair : multi_touch_points_) {
                    if (pair.second.valid) {
                        multitouch_callback_(pair.second);
                    }
                }
            }
    }
	LOG_INFO("touch: x=%d y=%d press=%d ID=%d\n",current_point_.x,current_point_.y,current_point_.pressure, current_point_.tracking_id);
}

