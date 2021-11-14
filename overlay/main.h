#pragma once
#include <openvr.h>
#include <Windows.h>

struct ControllerPipeData {
  short controllerId;
};

class PipeHelper {
 public:
  PipeHelper();

  bool ConnectAndSendPipe(const std::string& pipeName, ControllerPipeData data);

 private:
  HANDLE pipeHandle_;
};