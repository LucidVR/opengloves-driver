#pragma once

#undef _WINSOCKAPI_
#define _WINSOCKAPI_

#include <memory>

#include "Bones.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "DeviceProvider/DeviceProvider.h"
#include "openvr_driver.h"

// A device provider for a controller that should appear as a new device in SteamVR. The alternative is the hook device provider,
// which hooks skeletal input into an existing device.
class PhysicalDeviceProvider : public DeviceProvider {
 public:
  vr::EVRInitError Initialize() override;

  void Cleanup() override;

  const char* const* GetInterfaceVersions() override;

  bool ShouldBlockStandbyMode() override;

  void EnterStandby() override;

  void LeaveStandby() override;

 private:
  std::unique_ptr<DeviceDriver> InstantiateDeviceDriver(const VRDeviceConfiguration& configuration) const;

  std::unique_ptr<DeviceDriver> leftHand_;
  std::unique_ptr<DeviceDriver> rightHand_;
};