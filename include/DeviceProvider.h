#pragma once

#undef _WINSOCKAPI_
#define _WINSOCKAPI_

#include <memory>

#include "Bones.h"
#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "openvr_driver.h"

/**
This class instantiates all the device drivers you have, meaning if you've
created multiple drivers for multiple different controllers, this class will
create instances of each of those and inform OpenVR about all of your devices.

Take a look at the comment blocks for all the methods in IServerTrackedDeviceProvider
too.
**/
class DeviceProvider : public vr::IServerTrackedDeviceProvider {
 public:
  /**
  Initialize and add your drivers to OpenVR here.
  **/
  vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;

  /**
  Called right before your driver is unloaded.
  **/
  void Cleanup() override;

  /**
  Returns version of the openVR interface this driver works with.
  **/
  const char* const* GetInterfaceVersions() override;

  /**
  Called every frame. Update your drivers here.
  **/
  void RunFrame() override;

  /**
  Return true if standby mode should be blocked. False otherwise.
  **/
  bool ShouldBlockStandbyMode() override;

  /**
  Called when OpenVR goes into stand-by mode, so you can tell your devices to go into stand-by mode
  **/
  void EnterStandby() override;

  /**
  Called when OpenVR leaves stand-by mode.
  **/
  void LeaveStandby() override;

 private:
  std::unique_ptr<DeviceDriver> leftHand_;
  std::unique_ptr<DeviceDriver> rightHand_;
  /**
   * returns the configuration set in VRSettings for the device role given
   **/
  static VRDeviceConfiguration GetDeviceConfiguration(vr::ETrackedControllerRole role);

  std::unique_ptr<DeviceDriver> InstantiateDeviceDriver(VRDeviceConfiguration configuration, std::shared_ptr<BoneAnimator> boneAnimator) const;
};