#include "ForceFeedback.h"

#include "DriverLog.h"

FFBListener::FFBListener(std::function<void(VRFFBData_t)> callback, vr::ETrackedControllerRole role) : m_callback(callback), m_role(role) {
  std::string pipeName = "\\\\.\\pipe\\vrapplication\\ffb\\curl\\";
  pipeName.append((role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand) ? "right" : "left");
  m_pipe = std::make_unique<NamedPipeListener<VRFFBData_t>>(pipeName);
};

void FFBListener::Start() {
  m_pipe->StartListening([&](VRFFBData_t* data) { m_callback(*data); });
}

void FFBListener::Stop() { m_pipe->StopListening(); };