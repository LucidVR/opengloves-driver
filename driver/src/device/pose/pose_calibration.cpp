// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "pose_calibration.h"

#include "util/driver_math.h"

void PoseCalibration::StartCalibration(const vr::DriverPose_t &maintain_pose, CalibrationMethod method) {
  calibration_method_ = method;

  maintain_pose_ = maintain_pose;

  // make sure our device doesn't have any velocities (so it's fully stationary)
  maintain_pose_.vecVelocity[0] = 0;
  maintain_pose_.vecVelocity[1] = 0;
  maintain_pose_.vecVelocity[2] = 0;
  maintain_pose_.vecAngularVelocity[0] = 0;
  maintain_pose_.vecAngularVelocity[1] = 0;
  maintain_pose_.vecAngularVelocity[2] = 0;

  is_calibrating_ = true;
}

void PoseCalibration::CancelCalibration(CalibrationMethod method) {
  if (calibration_method_ == method) is_calibrating_ = false;
}

PoseConfiguration PoseCalibration::CompleteCalibration(
    const vr::TrackedDevicePose_t &controller_pose, const PoseConfiguration &pose_configuration, bool is_right_hand, CalibrationMethod method) {
  if (calibration_method_ != method) return pose_configuration;

  PoseConfiguration result{};

  is_calibrating_ = false;

  const vr::HmdVector3d_t new_position = MatrixToPosition(controller_pose.mDeviceToAbsoluteTracking);
  const vr::HmdQuaternion_t new_rotation = MatrixToOrientation(controller_pose.mDeviceToAbsoluteTracking);

  const vr::HmdVector3d_t last_position{
      maintain_pose_.vecPosition[0],
      maintain_pose_.vecPosition[1],
      maintain_pose_.vecPosition[2],
  };

  const vr::HmdQuaternion_t transform_orientation = -new_rotation * maintain_pose_.qRotation;
  result.offset_orientation = transform_orientation;

  const vr::HmdVector3d_t transform_position = (last_position - new_position) * -new_rotation;
  result.offset_position = transform_position;

  return result;
}

vr::DriverPose_t PoseCalibration::GetMaintainPose() const {
  return maintain_pose_;
}

bool PoseCalibration::IsCalibrating() const {
  return is_calibrating_;
}