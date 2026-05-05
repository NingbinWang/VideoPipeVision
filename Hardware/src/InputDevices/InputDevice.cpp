#include "InputDevice.h"
#include <thread>
#include <atomic>
#include <dirent.h>
#include <cstring>
#include <iostream>
#include <algorithm>
#include "Logger.h"

InputDevice::InputDevice() 
{
}

InputDevice::~InputDevice() 
{
    stopListening();
    closeDevice();
}

std::vector<InputDevice::DeviceInfo> InputDevice::enumerateDevices() {
    std::vector<DeviceInfo> devices;
    
    DIR* dir = opendir("/dev/input");
    if (!dir) {
        LOG_ERROR(" can 't open /dev/input\n");
        return devices;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strncmp(entry->d_name, "event", 5) == 0) {
            std::string path = std::string("/dev/input/") + entry->d_name;
            LOG_INFO("path = %s\n",path.c_str());
            int fd = open(path.c_str(), O_RDONLY);
            if (fd < 0) continue;
            
            DeviceInfo info;
            info.path = path;
            
            char name[256] = {0};
            if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0) {
                info.name = name;
            }
            
            char phys[256] = {0};
            if (ioctl(fd, EVIOCGPHYS(sizeof(phys)), phys) >= 0) {
                info.physical_path = phys;
            }
            
            if (ioctl(fd, EVIOCGID, &info.id) < 0) {
                memset(&info.id, 0, sizeof(info.id));
            }
            
            // 获取支持的事件类型
             unsigned long evbit[(EV_MAX + sizeof(unsigned long) * 8 - 1) / (sizeof(unsigned long) * 8)] = {0};
            if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) >= 0) {
                    for (unsigned int i = 0; i < EV_MAX && i < sizeof(evbit)*8; i++) {
                        unsigned int byte_index = i / (8 * sizeof(unsigned long));
                        unsigned int bit_index = i % (8 * sizeof(unsigned long));
                        if (evbit[byte_index] & (1UL << bit_index)) {
                            info.supported_events.push_back(i);
                        }
                    }
            }
            devices.push_back(info);
            close(fd);
        }
    }
    
    closedir(dir);
    return devices;
}

bool InputDevice::openDevice(const std::string& device_path) {
    if (isOpen()) {
        closeDevice();
    }
    
    fd_ = open(device_path.c_str(), O_RDONLY);
    if (fd_ < 0) {
        return false;
    }
    
    device_path_ = device_path;
    return true;
}

bool InputDevice::openDeviceByEventType(unsigned int event_type) {
    auto devices = enumerateDevices();
    for (const auto& device : devices) {
        if (std::find(device.supported_events.begin(), 
                    device.supported_events.end(), event_type) != 
            device.supported_events.end()) {
            return openDevice(device.path);
        }
    }
    return false;
}

void InputDevice::closeDevice() {
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
    device_path_.clear();
}

void InputDevice::startListening() {
    if (!isOpen() || listening_) {
        return;
    }
    
    listening_ = true;
    std::thread listener(&InputDevice::listeningThread, this);
    listener.detach();
}

void InputDevice::stopListening() {
    listening_ = false;
}

void InputDevice::listeningThread() {
    input_event ev;
    
    while (listening_) {
        if (readEvent(ev)) {
            processEvent(ev);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool InputDevice::readEvent(input_event& ev) {
    ssize_t n = read(fd_, &ev, sizeof(ev));
    return n == sizeof(ev);
}

void InputDevice::processEvent(const input_event& ev) {
    // 原始事件回调
    if (raw_event_callback_) {
        raw_event_callback_(ev);
    }
    
    // 分类处理事件
    switch (ev.type) {
        case EV_KEY:
            onKeyEvent(ev);
            break;
        case EV_ABS:
            onAbsEvent(ev);
            break;
        case EV_REL:
            onRelEvent(ev);
            break;
        case EV_SYN:
            onSynEvent(ev);
            break;
        default:
            break;
    }
    
    // 通用事件回调
    if (event_callback_) {
        event_callback_(ev);
    }
}

void InputDevice::onKeyEvent(const input_event& ev) {
    LOG_INFO("onKeyEvent code %d vlaue:%d \n",ev.code,ev.value);
}

void InputDevice::onAbsEvent(const input_event& ev) {
    LOG_INFO("onAbsEvent code %d vlaue:%d \n",ev.code,ev.value);
}

void InputDevice::onRelEvent(const input_event& ev) {
    LOG_INFO("onRelEvent code %d vlaue:%d \n",ev.code,ev.value);
}

void InputDevice::onSynEvent(const input_event& ev) {
    // 同步事件，通常不需要特殊处理
}

InputDevice::DeviceInfo InputDevice::getDeviceInfo() const {
    DeviceInfo info;
    if (!isOpen()) return info;
    
    info.path = device_path_;
    
    char name[256] = {0};
    if (ioctl(fd_, EVIOCGNAME(sizeof(name)), name) >= 0) {
        info.name = name;
    }
    
    char phys[256] = {0};
    if (ioctl(fd_, EVIOCGPHYS(sizeof(phys)), phys) >= 0) {
        info.physical_path = phys;
    }
    
    if (ioctl(fd_, EVIOCGID, &info.id) < 0) {
        memset(&info.id, 0, sizeof(info.id));
    }
    
// 获取支持的事件类型
        unsigned long evbit[(EV_MAX + sizeof(unsigned long) * 8 - 1) / (sizeof(unsigned long) * 8)] = {0};
        if (ioctl(fd_, EVIOCGBIT(0, sizeof(evbit)), evbit) >= 0) {
            for (unsigned int i = 0; i < EV_MAX && i < sizeof(evbit) * 8; i++) {
                unsigned int byte_index = i / (8 * sizeof(unsigned long));
                unsigned int bit_index = i % (8 * sizeof(unsigned long));
                if (byte_index < sizeof(evbit)/sizeof(unsigned long) && 
                    (evbit[byte_index] & (1UL << bit_index))) {
                    info.supported_events.push_back(i);
                }
            }
        }
    return info;
}