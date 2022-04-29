#include <DeviceProvider.h>
#include <Windows.h>

#include <string>

#include "DeviceDriver/KnuckleDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DriverLog.h"
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
    DriverLog("Could not create background process: %s", GetLastErrorAsString().c_str());

    return vr::VRInitError_Init_FileNotFound;
  }

  InitialiseDeviceDriver(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);
  InitialiseDeviceDriver(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

  return vr::VRInitError_None;
}

void DeviceProvider::InitialiseDeviceDriver(const vr::ETrackedControllerRole& role) {
  deviceConfigurations_[role] = GetDriverConfiguration(role);
  const VRDriverConfiguration& configuration = deviceConfigurations_.at(role);

  if (!configuration.enabled) {
    DriverLog("Not enabling device as it was disabled in configuration.");
    return;
  }

  devices_[role] = InstantiateDeviceDriver(configuration);
  const std::unique_ptr<DeviceDriver>& device = devices_.at(role);

  // this device hasn't previously been registered, so register it with SteamVR
  vr::VRServerDriverHost()->TrackedDeviceAdded(device->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, device.get());
}

std::unique_ptr<DeviceDriver> DeviceProvider::InstantiateDeviceDriver(const VRDriverConfiguration& configuration) const {
  switch (configuration.deviceConfiguration.deviceType) {
    case VRDeviceType::EmulatedKnuckles: {
      DriverLog("Using knuckles device driver");
      return std::make_unique<KnuckleDeviceDriver>(configuration.deviceConfiguration);
    }

    case VRDeviceType::LucidGloves: {
      DriverLog("Using lucidgloves device driver");
      return std::make_unique<LucidGloveDeviceDriver>(configuration.deviceConfiguration);
    }
  }
}

void DeviceProvider::Cleanup() {}

const char* const* DeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void DeviceProvider::HandleSettingsUpdate(const vr::ETrackedControllerRole& role) {
  const VRDriverConfiguration newConfiguration = GetDriverConfiguration(role);

  if (deviceConfigurations_.at(role) == newConfiguration) {
    DriverLog("Settings were updated, but opengloves did not need to update");
    return;
  }

  DriverLog(
      "A settings change was detected that affects: %s hand", role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left");

  // we have something that's updated.

  deviceConfigurations_[role] = newConfiguration;

  if (devices_.count(role) > 0) {
    if (newConfiguration.enabled) {
      DriverLog("Settings were updated, and attempting to update device");
      devices_.at(role)->UpdateDeviceConfiguration(newConfiguration.deviceConfiguration);
    } else {
      DriverLog("Settings were updated, and attempting to deactivate device as it was disabled");
      devices_.at(role)->DisableDevice();
    }
  } else {
    if (newConfiguration.enabled) {
      DriverLog("Settings were updated, and need to initialise new device");
      InitialiseDeviceDriver(role);
    }
  }
}

void DeviceProvider::RunFrame() {
  vr::VREvent_t pEvent;
  while (vr::VRServerDriverHost()->PollNextEvent(&pEvent, sizeof(pEvent))) {
    switch (pEvent.eventType) {
      case vr::EVREventType::VREvent_OtherSectionSettingChanged: {
        DriverLog("A settings change was detected that might affect our drivers...");

        HandleSettingsUpdate(vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
        HandleSettingsUpdate(vr::ETrackedControllerRole::TrackedControllerRole_LeftHand);

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
