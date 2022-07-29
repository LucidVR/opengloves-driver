#pragma once

#include "openvr_driver.h"

/*!
 * Represents a device that will in the end produce a new device in SteamVR, rather than trying to hook another device.
 */
class PhysicalDeviceProvider : vr::IServerTrackedDeviceProvider {
 public:
  vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;

  void Cleanup() override;

  const char* const* GetInterfaceVersions() override;

  void RunFrame() override;

  bool ShouldBlockStandbyMode() override;

  void EnterStandby() override;

  void LeaveStandby() override;
};