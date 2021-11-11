#include "ControllerDiscovery.h"

ControllerDiscovery::ControllerDiscovery(vr::ETrackedControllerRole role, std::function<void(ControllerDiscoveryPipeData)> callback)
    : role_(role), callback_(std::move(callback)) {
  std::string pipeName =
      R"(\\.\pipe\vrapplication\discovery\)" + std::string(role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");

  pipe_ = std::make_unique<NamedPipeListener<ControllerDiscoveryPipeData>>(pipeName);
};

void ControllerDiscovery::Start() {
  pipe_->StartListening([&](const ControllerDiscoveryPipeData* data) { callback_(*data); });
};

void ControllerDiscovery::Stop() const {
  pipe_->StopListening();
};