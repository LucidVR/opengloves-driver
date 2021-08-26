#pragma once

#include <memory>
#include <functional>
#include "openvr_driver.h"
#include "Util/NamedPipeListener.h"

struct ControllerDiscoveryPipeData_t {
  short controllerId;
};

class ControllerDiscovery {
 public:
  ControllerDiscovery(vr::ETrackedControllerRole role, std::function<void(ControllerDiscoveryPipeData_t)> callback);

  void Start();
  void Stop();

 private:
  vr::ETrackedControllerRole m_role;
  std::unique_ptr<NamedPipeListener<ControllerDiscoveryPipeData_t>> m_pipe;
  std::function<void(ControllerDiscoveryPipeData_t)> m_callback;
};