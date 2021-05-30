#pragma once

#include <windows.h>

#include <atomic>
#include <functional>
#include <thread>

#include "openvr_driver.h"

struct VRFFBData_t {
  short thumbCurl;
  short indexCurl;
  short middleCurl;
  short ringCurl;
  short pinkyCurl;

  vr::ETrackedControllerRole handedness;
};
typedef struct {
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  VRFFBData_t chRequest;
  DWORD cbRead;
  DWORD cbToWrite;
  std::function<void(VRFFBData_t)> callback;
} PIPEINST, *LPPIPEINST;

class FFBPipe {
 public:
  FFBPipe();
  bool Start(const std::function<void(VRFFBData_t)>& callback, vr::ETrackedControllerRole handedness);
  void Stop();

 private:
  void PipeListenerThread(const std::function<void(VRFFBData_t)>& callback, vr::ETrackedControllerRole handedness);
  void DisconnectAndClose();
  bool CreateAndConnectInstance(LPOVERLAPPED lpo);
  bool ConnectToNewClient(LPOVERLAPPED lpo);
  HANDLE m_hPipe;

  std::thread m_pipeThread;

  std::atomic<bool> m_listenerActive;
  std::atomic<bool> m_clientConnected;

  LPPIPEINST m_lpPipeInst;
};