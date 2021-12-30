#include "ForceFeedback.h"

#include <utility>

FFBListener::FFBListener(std::function<void(VRFFBData)> callback, const vr::ETrackedControllerRole role)
    : callback_(std::move(callback)), role_(role) {
  std::string pipeName = R"(\\.\pipe\vrapplication\ffb\curl\)";
  pipeName.append(role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");
  pipe_ = std::make_unique<NamedPipeListener<VRFFBData>>(pipeName);
};

void FFBListener::Start() {
  pipe_->StartListening([&](const VRFFBData* data) { callback_(*data); });
}

void FFBListener::Stop() const {
  pipe_->StopListening();
};