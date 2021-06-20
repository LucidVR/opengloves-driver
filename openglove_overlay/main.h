#pragma once
#include <openvr.h>
#include <windows.h>

#include <iostream>

struct ControllerPipeData {
  short controllerId;
};

class PipeHelper {
 public:
  PipeHelper();

  bool ConnectAndSendPipe(const std::string& pipeName, ControllerPipeData data);
  void Send();

 private:
  HANDLE m_pipeHandle;
};