#pragma once

#include <functional>
#include <memory>

#include "Util/NamedPipeListener.h"
#include "openvr_driver.h"

struct ControllerDiscoveryPipeData {
  short controllerId;
};

class ControllerDiscovery {
 public:
  ControllerDiscovery(vr::ETrackedControllerRole role, std::function<void(ControllerDiscoveryPipeData)> callback);

  void Start();
  void Stop() const;

 private:
  vr::ETrackedControllerRole _role;
  std::unique_ptr<NamedPipeListener<ControllerDiscoveryPipeData>> _pipe;
  std::function<void(ControllerDiscoveryPipeData)> _callback;
};