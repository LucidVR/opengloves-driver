#include "main.h"
#include <thread>
#include <chrono>
#include <atomic>


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

int32_t DiscoverController(vr::ETrackedControllerRole role) {
  while (appActive) {
    for (int32_t i = 1; i < vr::k_unMaxTrackedDeviceCount; i++) {
      vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(i);

      char thisManufacturer[1024];
      uint32_t err = vr::VRSystem()->GetStringTrackedDeviceProperty(
          i, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, thisManufacturer,
          sizeof(thisManufacturer));

      std::string sThisManufacturer(thisManufacturer);

      if (ourManufacturer == sThisManufacturer) continue;

      short deviceRole = vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(i);

      if (deviceRole == role) return i;
    }
    std::cout << "Could not find a controller... Sleeping" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void GetAndSendControllerId(vr::ETrackedControllerRole role) {
  int id = DiscoverController(role);

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

int main() {
  vr::EVRInitError error;
  VR_Init(&error, vr::VRApplication_Overlay);

  if (error == vr::EVRInitError::VRInitError_None) {
    std::cout << "Initialised..." << std::endl;

    std::thread leftControllerThread = std::thread(
        &GetAndSendControllerId, vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);

    std::thread rightControllerThread = std::thread(
        &GetAndSendControllerId, vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

    while (appActive) {
      vr::VREvent_t event;
      while (vr::VRSystem() && vr::VRSystem()->PollNextEvent(&event, sizeof event)) {
        switch (event.eventType) {
          case vr::VREvent_Quit:
            std::cout << "Received a shutdown command" << std::endl;

            appActive = false;
            vr::VRSystem()->AcknowledgeQuit_Exiting();

            leftControllerThread.join();
            rightControllerThread.join();
            vr::VR_Shutdown();
            return 1;
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  } else {
    std::cout << "Error initialising openvr: " << error << std::endl;
    vr::VR_Shutdown();
  }
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

    // Exit if an error other than ERROR_PIPE_BUSY occurs.

    if (GetLastError() != ERROR_PIPE_BUSY) {
        std::cout << "error!" << std::endl;
      return -1;
    }

    // All pipe instances are busy, so wait for 20 seconds.

    if (!WaitNamedPipe(pipeName.c_str(), 20000)) {
      printf("Could not open pipe: 20 second wait timed out.");
      return -1;
    } 
  }

  std::cout << "Writing to pipe..." << std::endl;
  DWORD dwWritten;

  WriteFile(m_pipeHandle, (LPCVOID)&data, sizeof(ControllerPipeData), &dwWritten, NULL);

  CloseHandle(m_pipeHandle);

  return true;
}