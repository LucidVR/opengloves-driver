#include "ForceFeedback/FFBIOBuffer.h"

#include <chrono>

#include "DriverLog.h"

FFBIOBuffer::FFBIOBuffer() : m_listenerActive(false), m_bufferHandle(0){};

void FFBIOBuffer::Start(const std::function<void(VRFFBData_t)> &callback,
                        vr::ETrackedControllerRole handedness) {
  m_listenerActive = true;
  m_bufferThread = std::thread(&FFBIOBuffer::BufferListenerThread, this, callback, handedness);
}

void FFBIOBuffer::BufferListenerThread(const std::function<void(VRFFBData_t)> &callback,
                                       vr::ETrackedControllerRole handedness) {
  while (vr::VRIOBuffer()->Open("/extensions/ffb/provider", vr::IOBufferMode_Read,
                                sizeof(VRFFBData_t), 2, &m_bufferHandle) != vr::IOBuffer_Success &&
         m_listenerActive) {
    DebugDriverLog("failed to open buffer, retrying, %i", sizeof(VRFFBData_t));
    // The buffer may not be open initially, so wait for it to be created
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  DebugDriverLog("Success opening buffer");
  while (m_listenerActive) {
    uint32_t unRead;
    VRFFBData_t ffbBufferData;

    vr::EIOBufferError err =
        vr::VRIOBuffer()->Read(m_bufferHandle, &ffbBufferData, sizeof(ffbBufferData), &unRead);

    if (err == vr::IOBuffer_Success) {
      if (unRead > 0 && ffbBufferData.handedness == handedness) callback(ffbBufferData);
    } else if (err == vr::IOBuffer_PathDoesNotExist) {
      DriverLog("FFB Path does not exist. Sleeping for 1000ms before trying again.");
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }
}

void FFBIOBuffer::CloseBuffer() { vr::VRIOBuffer()->Close(m_bufferHandle); }

void FFBIOBuffer::Stop() {
  m_listenerActive = false;
  m_bufferThread.join();
  CloseBuffer();
}
