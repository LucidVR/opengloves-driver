// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <map>
#include <memory>

#include "openvr_driver.h"
#include "opengloves_interface.h"
#include "device/drivers/device_driver.h"

/*!
 * Represents a device that will in the end produce a new device in SteamVR, rather than trying to hook another device.
 */
class PhysicalDeviceProvider : public vr::IServerTrackedDeviceProvider {
 public:
  vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;

  void Cleanup() override;

  const char* const* GetInterfaceVersions() override;

  void RunFrame() override;

  bool ShouldBlockStandbyMode() override;

  void EnterStandby() override;

  void LeaveStandby() override;

 private:
  std::unique_ptr<og::Server> ogserver_;

  std::map<vr::ETrackedControllerRole, std::unique_ptr<IDeviceDriver>> device_drivers_;
};