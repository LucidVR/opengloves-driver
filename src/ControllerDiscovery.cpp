#include "ControllerDiscovery.h"

ControllerDiscovery::ControllerDiscovery(vr::ETrackedControllerRole role, std::function<void(ControllerDiscoveryPipeData_t)> callback)
    : m_role(role), m_callback(std::move(callback)) {
  std::string pipeName = "\\\\.\\pipe\\vrapplication\\discovery\\" + std::string(role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");

  m_pipe = std::make_unique<NamedPipeListener<ControllerDiscoveryPipeData_t>>(pipeName);
};

void ControllerDiscovery::Start() {
  m_pipe->StartListening([&](ControllerDiscoveryPipeData_t* data) { m_callback(*data); });
};

void ControllerDiscovery::Stop() { m_pipe->StopListening(); };