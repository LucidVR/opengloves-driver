#include "ForceFeedback.h"

#include <chrono>

#include "DriverLog.h"

static std::string GetLastErrorAsString() {
  // Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return std::string();  // No error message has been recorded
  }

  LPSTR messageBuffer = nullptr;

  // Ask Win32 to give us the string version of that message ID.
  // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  // Copy the error message into a std::string.
  std::string message(messageBuffer, size);

  // Free the Win32's string's buffer.
  LocalFree(messageBuffer);

  return message;
}

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