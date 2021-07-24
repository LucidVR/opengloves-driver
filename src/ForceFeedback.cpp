#include "ForceFeedback.h"

#include <chrono>

#include "DriverLog.h"

FFBListener::FFBListener(std::function<void(VRFFBData_t)> callback, vr::ETrackedControllerRole role) : m_callback(callback), m_role(role) {
  std::string pipeName = "\\\\.\\pipe\\vrapplication\\ffb\\curl\\";
  pipeName.append((role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand) ? "right" : "left");
  m_pipe = std::make_unique<NamedPipeUtil>(pipeName, sizeof(VRFFBData_t));
};

void FFBListener::Start() {
  m_pipe->Start([&](LPVOID data) {
    VRFFBData_t *ffbData = (VRFFBData_t *)data;

    VRFFBData_t result(*ffbData);
    m_callback(result);
  });
}

void FFBListener::Stop() { m_pipe->Stop(); };