#pragma once

#include <windows.h>

#include <atomic>
#include <functional>
#include <thread>

#include "openvr_driver.h"

struct ControllerPipeData {
  short controllerId;
};
typedef struct {
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  ControllerPipeData chRequest;
  DWORD cbRead;
  DWORD cbToWrite;
  std::function<void(ControllerPipeData)> callback;
} PIPEINST, *LPPIPEINST;

class ControllerDiscoveryPipe {
 public:
  ControllerDiscoveryPipe();
  bool Start(const std::function<void(ControllerPipeData)>& callback, vr::ETrackedControllerRole role);
  void Stop();

 private:
  void PipeListenerThread(const std::function<void(ControllerPipeData)>& callback,
                          vr::ETrackedControllerRole role);
  void DisconnectAndClose();
  bool CreateAndConnectInstance(LPOVERLAPPED lpo, std::string& pipeName);
  bool ConnectToNewClient(LPOVERLAPPED lpo);
  HANDLE m_hPipe;

  std::thread m_pipeThread;

  std::atomic<bool> m_listenerActive;
  std::atomic<bool> m_clientConnected;
  LPPIPEINST m_lpPipeInst;
};