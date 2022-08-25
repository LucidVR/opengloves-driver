#pragma once

#include <functional>

#include "opengloves_interface.h"

class IDeviceDiscoverer {
 public:
  virtual void StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) = 0;
};