#pragma once
#include <openvr.h>
#include <Windows.h>

struct ControllerPipeData {
  short controllerId;
};

class PipeHelper {
 public:
  PipeHelper(const std::string& pipeName);

  bool SendPipe(const ControllerPipeData& data);

  void WaitCreatePipe();

 private:
  std::string pipeName_;
  HANDLE pipeHandle_;
};