#pragma once
#include <Windows.h>
#include <openvr.h>

struct ControllerPipeData {
  short controllerId;
};

class PipeHelper {
 public:
  PipeHelper(const std::string& pipeName);

  bool SendPipe(const ControllerPipeData& data);

  void WaitCreatePipe();

  void ClosePipe();

 private:
  std::string pipeName_;
  HANDLE pipeHandle_;
};