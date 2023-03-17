// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "device_driver.h"
#include "opengloves_interface.h"
#include "openvr_driver.h"

enum KnuckleDeviceComponentIndex {
  kKnuckleDeviceComponentIndex_SystemClick = 0,
  kKnuckleDeviceComponentIndex_SystemTouch,
  kKnuckleDeviceComponentIndex_TriggerClick,
  kKnuckleDeviceComponentIndex_TriggerValue,
  kKnuckleDeviceComponentIndex_TrackpadX,
  kKnuckleDeviceComponentIndex_TrackpadY,
  kKnuckleDeviceComponentIndex_TrackpadTouch,
  kKnuckleDeviceComponentIndex_TrackpadForce,
  kKnuckleDeviceComponentIndex_GripTouch,
  kKnuckleDeviceComponentIndex_GripForce,
  kKnuckleDeviceComponentIndex_GripValue,
  kKnuckleDeviceComponentIndex_ThumbstickClick,
  kKnuckleDeviceComponentIndex_ThumbstickTouch,
  kKnuckleDeviceComponentIndex_ThumbstickX,
  kKnuckleDeviceComponentIndex_ThumbstickY,
  kKnuckleDeviceComponentIndex_AClick,
  kKnuckleDeviceComponentIndex_ATouch,
  kKnuckleDeviceComponentIndex_BClick,
  kKnuckleDeviceComponentIndex_BTouch,
  kKnuckleDeviceComponentIndex_FingerIndex,
  kKnuckleDeviceComponentIndex_FingerMiddle,
  kKnuckleDeviceComponentIndex_FingerRing,
  kKnuckleDeviceComponentIndex_FingerPinky,

  kKnuckleDeviceComponentIndex_Skeleton,
  kKnuckleDeviceComponentIndex_Count
};

class KnuckleDeviceDriver : public IDeviceDriver {
 public:
  explicit KnuckleDeviceDriver(vr::ETrackedControllerRole role);

  void SetDeviceDriver(std::unique_ptr<og::IDevice> device) override;

  vr::EVRInitError Activate(uint32_t unObjectId) override;

  void Deactivate() override;

  void EnterStandby() override;

  void *GetComponent(const char *pchComponentNameAndVersion) override;

  void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;

  vr::DriverPose_t GetPose() override;

  std::string GetSerialNumber() override;

  bool IsActive() override;

  ~KnuckleDeviceDriver();

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  vr::ETrackedControllerRole role_;

  std::atomic<bool> is_active_;
};