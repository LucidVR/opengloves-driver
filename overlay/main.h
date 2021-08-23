#pragma once
#include <openvr.h>
#include <windows.h>

struct ControllerPipeData {
  short controllerId;
};

class PipeHelper {
 public:
  PipeHelper();

  bool ConnectAndSendPipe(const std::string& pipeName, ControllerPipeData data);

 private:
  HANDLE m_pipeHandle;
};