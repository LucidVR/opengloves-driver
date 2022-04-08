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
  // we don't want to initialise devices that have already been created
  if (devices_.count(role) > 0) {
    DriverLog("Attempting to intialise an previously initialised device. Will not initialise");

    return;
  }

  deviceConfigurations_[role] = GetDriverConfiguration(role);
  const VRDriverConfiguration& configuration = deviceConfigurations_.at(role);

  if (!configuration.enabled) {
    DriverLog(
        "Not loading device for %s hand as it was disabled in settings.",
        role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");

    return;
  }

  devices_[role] = InstantiateDeviceDriver(configuration);
  const std::unique_ptr<DeviceDriver>& device = devices_.at(role);

  // this device hasn't previously been registered, so register it with SteamVR
  vr::VRServerDriverHost()->TrackedDeviceAdded(device->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, device.get());
}

std::unique_ptr<CommunicationManager> DeviceProvider::InstantiateCommunicationManager(const VRDriverConfiguration& configuration) const {
  std::unique_ptr<EncodingManager> encodingManager;

  switch (configuration.encodingConfiguration.encodingProtocol) {
    case VREncodingProtocol::Legacy: {
      DriverLog("Using legacy encoding");
      encodingManager = std::make_unique<LegacyEncodingManager>(configuration.encodingConfiguration);

      break;
    }

    case VREncodingProtocol::Alpha: {
      DriverLog("Using alpha encoding");
      encodingManager = std::make_unique<AlphaEncodingManager>(configuration.encodingConfiguration);

      break;
    }
  }

  switch (configuration.communicationConfiguration.communicationProtocol) {
    case VRCommunicationProtocol::NamedPipe: {
      DriverLog("Using named pipe communication");
      return std::make_unique<NamedPipeCommunicationManager>(configuration.communicationConfiguration);
    }

    case VRCommunicationProtocol::BtSerial: {
      DriverLog("Using bluetooth serial communication");
      return std::make_unique<BTSerialCommunicationManager>(configuration.communicationConfiguration, std::move(encodingManager));
    }

    case VRCommunicationProtocol::Serial: {
      DriverLog("Using usb serial communication");
      return std::make_unique<SerialCommunicationManager>(configuration.communicationConfiguration, std::move(encodingManager));
    }
  }
}

std::unique_ptr<DeviceDriver> DeviceProvider::InstantiateDeviceDriver(const VRDriverConfiguration& configuration) const {
  std::unique_ptr<CommunicationManager> communicationManager = InstantiateCommunicationManager(configuration);
  std::unique_ptr<BoneAnimator> boneAnimator = std::make_unique<BoneAnimator>(GetDriverPath() + R"(\resources\anims\glove_anim.glb)");

  switch (configuration.deviceConfiguration.deviceType) {
    case VRDeviceType::EmulatedKnuckles: {
      DriverLog("Using knuckles device driver");
      return std::make_unique<KnuckleDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), configuration.deviceConfiguration);
    }

    case VRDeviceType::LucidGloves: {
      return std::make_unique<LucidGloveDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), configuration.deviceConfiguration);
    }
  }
}

void DeviceProvider::Cleanup() {}

const char* const* DeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void DeviceProvider::HandleSettingsUpdate(const vr::ETrackedControllerRole& role) {
  const VRDriverConfiguration newConfiguration = GetDriverConfiguration(role);

  // we don't want to update if there's nothing to update
  if (devices_.count(role) > 0 && deviceConfigurations_.at(role) == newConfiguration) {
    DriverLog(
        "Settings were updated, but found no reason to update role: %s Hand",
        role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "Right" : "Left");

    return;
  }

  std::unique_ptr<DeviceDriver> newDevice = InstantiateDeviceDriver(newConfiguration);
  devices_.at(role)->Deactivate();

  newDevice->Activate(devices_.at(role)->GetDeviceId());


  devices_.at(role).swap(newDevice);

  DriverLog(
      "Settings were updated, and successfully updated %s hand",
      role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");
}

void DeviceProvider::RunFrame() {
  vr::VREvent_t pEvent;
  while (vr::VRServerDriverHost()->PollNextEvent(&pEvent, sizeof(pEvent))) {
    switch (pEvent.eventType) {
      case vr::EVREventType::VREvent_OtherSectionSettingChanged: {
        DriverLog("A setting change was detected.");

        HandleSettingsUpdate(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
        HandleSettingsUpdate(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

        break;
      }

      default: {
        for (auto const& ptr : devices_) {
          if (ptr.second->IsActive()) ptr.second->OnEvent(pEvent);
        }
      }
    }
  }
}

bool DeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}
