#include "ForceFeedback.h"

#include <utility>

FFBListener::FFBListener(std::function<void(VRFFBData)> callback, const vr::ETrackedControllerRole role)
    : _callback(std::move(callback)), _role(role) {
  std::string pipeName = R"(\\.\pipe\vrapplication\ffb\curl\)";
  pipeName.append(role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");
  _pipe = std::make_unique<NamedPipeListener<VRFFBData>>(pipeName);
};

void FFBListener::Start() {
  _pipe->StartListening([&](const VRFFBData* data) { _callback(*data); });
}

void FFBListener::Stop() const {
  _pipe->StopListening();
};