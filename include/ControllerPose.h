#pragma once
#include <openvr_driver.h>
#include <memory>
#include "DeviceConfiguration.h"
#include "ControllerDiscovery.h"

class ControllerPose {
 public:
  ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer,
                 VRPoseConfiguration_t poseConfiguration);
  vr::DriverPose_t UpdatePose();

 private:
  uint32_t m_shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

  VRPoseConfiguration_t m_poseConfiguration;

  vr::ETrackedControllerRole m_shadowDeviceOfRole = vr::TrackedControllerRole_Invalid;

  std::string m_thisDeviceManufacturer;

  bool IsOtherRole(int32_t test);
  std::unique_ptr<ControllerDiscoveryPipe> m_controllerDiscoverer;
};