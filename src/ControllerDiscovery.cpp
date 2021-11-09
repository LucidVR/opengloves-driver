#include "ControllerDiscovery.h"

ControllerDiscovery::ControllerDiscovery(vr::ETrackedControllerRole role, std::function<void(ControllerDiscoveryPipeData)> callback)
    : _role(role), _callback(std::move(callback)) {
  std::string pipeName =
      R"(\\.\pipe\vrapplication\discovery\)" + std::string(role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");

  _pipe = std::make_unique<NamedPipeListener<ControllerDiscoveryPipeData>>(pipeName);
};

void ControllerDiscovery::Start() {
  _pipe->StartListening([&](const ControllerDiscoveryPipeData* data) { _callback(*data); });
};

void ControllerDiscovery::Stop() const {
  _pipe->StopListening();
};