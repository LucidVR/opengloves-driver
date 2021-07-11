#include "main.h"

#include <atomic>
#include <chrono>
#include <thread>

std::atomic<bool> appActive = true;

const std::string ourManufacturer = "LucasVRTech&Danwillm";

std::string GetLastErrorAsString() {
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return std::string();
  }

  LPSTR messageBuffer = nullptr;

  size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0,
      NULL);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

void GetAndSendControllerId(int id, vr::ETrackedControllerRole role) {
  std::unique_ptr<PipeHelper> pipeHelper = std::make_unique<PipeHelper>();
  std::string pipeName;

  if (role == vr::ETrackedControllerRole::TrackedControllerRole_LeftHand) {
    pipeName = "\\\\.\\pipe\\vrapplication\\discovery\\left";
  } else {
    pipeName = "\\\\.\\pipe\\vrapplication\\discovery\\right";
  }

  ControllerPipeData data;
  data.controllerId = id;

  pipeHelper->ConnectAndSendPipe(pipeName, data);
}

void DiscoverController(vr::ETrackedControllerRole role) {
  int lastFound = -1;
  int curFound = -1;

  while (appActive) {
    for (int32_t i = 1; i < vr::k_unMaxTrackedDeviceCount; i++) {
      char thisManufacturer[1024];
      uint32_t err = vr::VRSystem()->GetStringTrackedDeviceProperty(
          i, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, thisManufacturer,
          sizeof(thisManufacturer));

      std::string sThisManufacturer(thisManufacturer);

      if (ourManufacturer == sThisManufacturer) continue;

      short deviceRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(i);

      int32_t controllerHint = vr::VRSystem()->GetInt32TrackedDeviceProperty(
          i, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32);

      if (controllerHint == role) {
        curFound = i;
        break;
      }
      
      int controllerType = vr::VRSystem()->GetInt32TrackedDeviceProperty(
          i, vr::ETrackedDeviceProperty::Prop_DeviceClass_Int32);

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
    std::thread leftControllerThread = std::thread(
        &DiscoverController, vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);

    std::thread rightControllerThread = std::thread(
        &DiscoverController, vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

    while (appActive) {
      vr::VREvent_t event;
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

PipeHelper::PipeHelper() {}

bool PipeHelper::ConnectAndSendPipe(const std::string& pipeName, ControllerPipeData data) {
  while (1) {
    m_pipeHandle = CreateFile(pipeName.c_str(),  // pipe name
                              GENERIC_READ |     // read and write access
                                  GENERIC_WRITE,
                              0,              // no sharing
                              NULL,           // default security attributes
                              OPEN_EXISTING,  // opens existing pipe
                              0,              // default attributes
                              NULL);          // no template file

    if (m_pipeHandle != INVALID_HANDLE_VALUE) break;

    if (GetLastError() != ERROR_PIPE_BUSY) {
      return -1;
    }

    if (!WaitNamedPipe(pipeName.c_str(), 1000)) {
      return -1;
    }
  }

  DWORD dwWritten;

  WriteFile(m_pipeHandle, (LPCVOID)&data, sizeof(ControllerPipeData), &dwWritten, NULL);

  CloseHandle(m_pipeHandle);

  return true;
}