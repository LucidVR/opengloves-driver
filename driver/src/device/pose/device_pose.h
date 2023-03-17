// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <atomic>
#include <memory>

#include "device/configuration/device_configuration.h"
#include "openvr_driver.h"
#include "pose_calibration.h"

class DevicePose {
 public:
  DevicePose(vr::ETrackedControllerRole role);

  vr::DriverPose_t UpdatePose() const;

  void StartCalibration(CalibrationMethod method) const;

  void CompleteCalibration(CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method) const;

  bool IsCalibrating() const;

 private:
  PoseConfiguration configuration_{};
  vr::ETrackedControllerRole role_;

  std::unique_ptr<PoseCalibration> calibration_;

  std::atomic<uint32_t> controller_id_ = 0;

  static vr::TrackedDevicePose_t GetControllerPose(uint32_t controller_id);
};