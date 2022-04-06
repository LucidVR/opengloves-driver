#include <DeviceProvider.h>
#include <Windows.h>

#include <algorithm>
#include <string>

#include "Communication/BTSerialCommunicationManager.h"
#include "Communication/NamedPipeCommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"
#include "DeviceDriver/KnuckleDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DriverLog.h"
#include "Encode/AlphaEncodingManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "Util/Quaternion.h"
#include "Util/Windows.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "?"
#endif

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  if (const vr::EVRInitError initError = InitServerDriverContext(pDriverContext); initError != vr::EVRInitError::VRInitError_None) return initError;

  VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext)
  InitDriverLog(vr::VRDriverLog());

  // this won't print if running in release
  DebugDriverLog("OpenGloves is running in DEBUG mode");

  const std::string driverPath = GetDriverPath();
  DriverLog("Path to DLL: %s", driverPath.c_str());

  const std::string commitHash = GIT_COMMIT_HASH;
  DriverLog("Built from: %s", commitHash.substr(0, 10).c_str());

  // Create background process for the overlay (used for finding controllers to bind to for tracking)
  if (!CreateBackgroundProcess(driverPath + R"(\bin\win64\openglove_overlay.exe)")) {
    DriverLog("Could not create background process: %c", GetLastErrorAsString().c_str());

    return vr::VRInitError_Init_FileNotFound;
  }

  InitialiseDeviceDriver(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
  InitialiseDeviceDriver(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

  return vr::VRInitError_None;
}

void DeviceProvider::InitialiseDeviceDriver(const vr::ETrackedControllerRole& role) {
  const std::string driverPath = GetDriverPath();

  const VRDriverConfiguration configuration = GetDeviceConfiguration(role);

  const auto boneAnimator = std::make_shared<BoneAnimator>(driverPath + R"(\resources\anims\glove_anim.glb)");

  // if we're re-initializing a device then make sure to store it's relevant properties
  int deviceId = -1;
  if (devices_.count(role) > 0 && devices_.at(role)->IsActive()) deviceId = devices_.at(role)->GetDeviceId();

  if (configuration.enabled) devices_.insert_or_assign(role, InstantiateDeviceDriver(configuration, boneAnimator));

  std::unique_ptr<DeviceDriver>& device = devices_.at(role);

  if (deviceId > 0) {
    // this device has previously been registered with SteamVR, so re-initialize it
    device->Activate(deviceId);
  } else {
    // this device hasn't previously been registered, so register it with SteamVR
    vr::VRServerDriverHost()->TrackedDeviceAdded(device->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, device.get());
  }
}

std::unique_ptr<DeviceDriver> DeviceProvider::InstantiateDeviceDriver(
    VRDriverConfiguration configuration, std::shared_ptr<BoneAnimator> boneAnimator) const {
  std::unique_ptr<CommunicationManager> communicationManager;
  std::unique_ptr<EncodingManager> encodingManager;

  const bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;

  switch (configuration.encodingProtocol) {
    case VREncodingProtocol::Alpha: {
      DriverLog("Using alpha encoding");
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_alphaEncodingSettingsSection, "max_analog_value");
      encodingManager = std::make_unique<AlphaEncodingManager>(maxAnalogValue);
      break;
    }
    default:
      DriverLog("No encoding protocol set. Using legacy.");
    case VREncodingProtocol::Legacy: {
      DriverLog("Using legacy encoding");
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_legacyEncodingSettingsSection, "max_analog_value");
      encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
      break;
    }
  }

  switch (configuration.communicationProtocol) {
    case VRCommunicationProtocol::NamedPipe: {
      DriverLog("Using named pipe communication");
      const std::string path = R"(\\.\pipe\vrapplication\input\glove\$version\)" + std::string(isRightHand ? "right" : "left");
      VRNamedPipeInputConfiguration namedPipeConfiguration(path);
      communicationManager = std::make_unique<NamedPipeCommunicationManager>(namedPipeConfiguration, configuration);
      break;
    }
    case VRCommunicationProtocol::BtSerial: {
      DriverLog("Using bluetooth serial communication");
      char name[248];
      vr::VRSettings()->GetString(c_btserialCommunicationSettingsSection, isRightHand ? "right_name" : "left_name", name, sizeof name);
      VRBTSerialConfiguration btSerialSettings(name);
      communicationManager = std::make_unique<BTSerialCommunicationManager>(std::move(encodingManager), btSerialSettings, configuration);
      break;
    }
    default:
      DriverLog("No communication protocol set. Using serial.");
    case VRCommunicationProtocol::Serial: {
      DriverLog("Using serial communication");
      char port[16];

      vr::VRSettings()->GetString(c_serialCommunicationSettingsSection, isRightHand ? "right_port" : "left_port", port, sizeof port);
      const int baudRate = vr::VRSettings()->GetInt32(c_serialCommunicationSettingsSection, "baud_rate");
      VRSerialConfiguration serialSettings(port, baudRate);

      communicationManager = std::make_unique<SerialCommunicationManager>(std::move(encodingManager), serialSettings, configuration);
      break;
    }
  }

  switch (configuration.deviceDriver) {
    case VRDeviceType::EmulatedKnuckles: {
      DriverLog("Using knuckles device driver");
      char serialNumber[32];
      vr::VRSettings()->GetString(
          c_knuckleDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof serialNumber);
      bool approximateThumb = vr::VRSettings()->GetBool(c_knuckleDeviceSettingsSection, "approximate_thumb");
      return std::make_unique<KnuckleDeviceDriver>(
          std::move(communicationManager), std::move(boneAnimator), serialNumber, approximateThumb, configuration);
    }

    default:
      DriverLog("No device driver selected. Using lucidgloves.");
    case VRDeviceType::LucidGloves: {
      DriverLog("Using lucidglove device driver");
      char serialNumber[32];
      vr::VRSettings()->GetString(
          c_lucidGloveDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof serialNumber);

      return std::make_unique<LucidGloveDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration);
    }
  }
}

void DeviceProvider::Cleanup() {}

const char* const* DeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame() {
  vr::VREvent_t pEvent;
  while (vr::VRServerDriverHost()->PollNextEvent(&pEvent, sizeof(pEvent))) {
    switch (pEvent.eventType) {
      case vr::EVREventType::VREvent_OtherSectionSettingChanged: {
        // settings have changed. Restart device drivers
      }

      default: {
        if (leftHand_ && leftHand_->IsActive()) leftHand_->OnEvent(pEvent);
        if (rightHand_ && rightHand_->IsActive()) rightHand_->OnEvent(pEvent);
      }
    }
  }
}

bool DeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}
