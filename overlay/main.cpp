#include "main.h"

#include <atomic>
#include <chrono>
#include <thread>

std::atomic<bool> appActive = true;

const std::string ourManufacturer = "LucidVR";

std::string GetLastErrorAsString() {
  const DWORD errorMessageId = ::GetLastError();
  if (errorMessageId == 0) {
    return std::string();
  }

  LPSTR messageBuffer = nullptr;

  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      errorMessageId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&messageBuffer),
      0,
      nullptr);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

void GetAndSendControllerId(const vr::TrackedDeviceIndex_t id, const vr::ETrackedControllerRole role) {
  const auto pipeHelper = std::make_unique<PipeHelper>();
  std::string pipeName;

  if (role == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand) {
    pipeName = R"(\\.\pipe\vrapplication\discovery\left)";
  } else {
    pipeName = R"(\\.\pipe\vrapplication\discovery\right)";
  }

  ControllerPipeData data{};
  data.controllerId = id;

  pipeHelper->ConnectAndSendPipe(pipeName, data);
}

void DiscoverController(const vr::ETrackedControllerRole role) {
  vr::TrackedDeviceIndex_t lastFound = -1;
  vr::TrackedDeviceIndex_t curFound = -1;

  while (appActive) {
    for (vr::TrackedDeviceIndex_t i = 1; i < vr::k_unMaxTrackedDeviceCount; i++) {
      char thisManufacturer[1024];
      uint32_t err = vr::VRSystem()->GetStringTrackedDeviceProperty(
          i, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, thisManufacturer, sizeof thisManufacturer);

      std::string sThisManufacturer(thisManufacturer);

      if (ourManufacturer == sThisManufacturer) continue;

      const short deviceRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(i);

      const int32_t controllerHint = vr::VRSystem()->GetInt32TrackedDeviceProperty(i, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32);

      if (controllerHint == role) {
        curFound = i;
        break;
      }

      const int controllerType = vr::VRSystem()->GetInt32TrackedDeviceProperty(i, vr::ETrackedDeviceProperty::Prop_DeviceClass_Int32);

      if (controllerType == vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker ||
          controllerType == vr::ETrackedDeviceClass::TrackedDeviceClass_Controller) {
        if (role == deviceRole) {
          curFound = i;
        }
      }
    }

    if (curFound != lastFound) {
      GetAndSendControllerId(curFound, role);
      lastFound = curFound;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
  vr::EVRInitError error;
  VR_Init(&error, vr::VRApplication_Background);

  if (error == vr::EVRInitError::VRInitError_None) {
    auto leftControllerThread = std::thread(&DiscoverController, vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);

    auto rightControllerThread = std::thread(&DiscoverController, vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

    while (appActive) {
      vr::VREvent_t event{};
      while (vr::VRSystem() && vr::VRSystem()->PollNextEvent(&event, sizeof event)) {
        switch (event.eventType) {
          case vr::VREvent_Quit:

            appActive = false;
            vr::VRSystem()->AcknowledgeQuit_Exiting();

            leftControllerThread.join();
            rightControllerThread.join();
            vr::VR_Shutdown();
            return 0;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  } else {
    vr::VR_Shutdown();
  }
  return 0;
}

PipeHelper::PipeHelper() : pipeHandle_(nullptr) {}

bool PipeHelper::ConnectAndSendPipe(const std::string& pipeName, ControllerPipeData data) {
  while (true) {
    pipeHandle_ = CreateFile(
        pipeName.c_str(),              // pipe name
        GENERIC_READ | GENERIC_WRITE,  // read and write access
        0,                             // no sharing
        nullptr,                       // default security attributes
        OPEN_EXISTING,                 // opens existing pipe
        0,                             // default attributes
        nullptr);                      // no template file

    if (pipeHandle_ != INVALID_HANDLE_VALUE) break;

    if (GetLastError() != ERROR_PIPE_BUSY) {
      return false;
    }

    if (!WaitNamedPipe(pipeName.c_str(), 1000)) {
      return false;
    }
  }

  DWORD dwWritten;

  WriteFile(pipeHandle_, &data, sizeof(ControllerPipeData), &dwWritten, nullptr);

  CloseHandle(pipeHandle_);

  return true;
}