// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "openvr_driver.h"

#include "device/configuration/device_configuration.h"

enum CalibrationMethod {
  kCalibrationMethod_Hardware,
  kCalibrationMethod_Ui,
  kCalibrationMethod_None,
};


class PoseCalibration {
 public:
  void StartCalibration(const vr::DriverPose_t& maintain_pose, CalibrationMethod method);

  PoseConfiguration CompleteCalibration(
      const vr::TrackedDevicePose_t& controller_pose, const PoseConfiguration& pose_configuration, bool is_right_hand, CalibrationMethod method);

  void CancelCalibration(CalibrationMethod method);

  bool IsCalibrating() const;

  vr::DriverPose_t GetMaintainPose() const;

 private:
  vr::DriverPose_t maintain_pose_;
  bool is_calibrating_;
  CalibrationMethod calibration_method_;
};