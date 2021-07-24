#include "ControllerDiscovery.h"

#include "DriverLog.h"

ControllerDiscovery::ControllerDiscovery(vr::ETrackedControllerRole role, std::function<void(ControllerDiscoveryPipeData_t)> callback)
    : m_role(role), m_callback(std::move(callback)) {

  std::string pipeName = "\\\.\\pipe\\vrapplication\\discovery\\" + std::string(role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");

  m_pipe = std::make_unique<NamedPipeUtil>(pipeName, sizeof(ControllerDiscoveryPipeData_t));
};

void ControllerDiscovery::Start() {
  m_pipe->Start([&](LPVOID data) {
    ControllerDiscoveryPipeData_t *controllerPipeData = (ControllerDiscoveryPipeData_t *)data;

    ControllerDiscoveryPipeData_t result(*controllerPipeData);
    m_callback(result);
  });
};

void ControllerDiscovery::Stop() { m_pipe->Stop(); };