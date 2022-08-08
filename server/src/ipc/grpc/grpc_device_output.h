#pragma once

#include <functional>

class GRPCDeviceOutput {
 public:
  GRPCDeviceOutput(std::function<void()> on_requested_get_info) {};

 private:
};