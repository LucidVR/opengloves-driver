#pragma once

#include <functional>
#include "openvr_driver.h"
#include <thread>
#include <atomic>

struct VRFFBData_t {
    short thumbCurl;
    short indexCurl;
    short middleCurl;
    short ringCurl;
    short pinkyCurl;

    vr::ETrackedControllerRole handedness;
};

/***
 * This class implements force feedback using the (now deprecated) IVRIOBuffer OpenVR interface.
 * https://github.com/ValveSoftware/openvr/issues/1543
 * Applications can create their own buffer (with path "/extensions/ffb/provider") which this driver will then open and listen to.
 */

class FFBIOBuffer {
public:
    FFBIOBuffer();
    void Start(const std::function<void(VRFFBData_t)>& callback, vr::ETrackedControllerRole handedness);
    void Stop();
private:
    void BufferListenerThread(const std::function<void(VRFFBData_t)>& callback, vr::ETrackedControllerRole handedness);
    void CloseBuffer();

    vr::IOBufferHandle_t m_bufferHandle;
    std::thread m_bufferThread;

    std::atomic<bool> m_listenerActive;
};