// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "device_pose.h"

#include "nlohmann/json.hpp"
#include "services/driver_external.h"
#include "services/driver_internal.h"
#include "util/driver_log.h"
#include "util/driver_math.h"

static DriverInternalServer& internal_server = DriverInternalServer::GetInstance();
static DriverExternalServer& external_server = DriverExternalServer::GetInstance();

DevicePose::DevicePose(vr::ETrackedControllerRole role) : role_(role), calibration_(std::make_unique<PoseCalibration>()) {
  configuration_ = GetPoseConfiguration(role);

  internal_server.AddTrackingReferenceRequestCallback([&](const TrackingReferenceResult& result) {
    if (result.role == role_) {
      DriverLog(
          "Controller that %s hand is tracking from has been updated to id: %i",
          role_ == vr::TrackedControllerRole_RightHand ? "right" : "left",
          result.controller_id);
      controller_id_ = result.controller_id;
    }
  });

  external_server.RegisterFunctionCallback(
      std::string("pose_calibration/") + std::string(role == vr::TrackedControllerRole_LeftHand ? "left" : "right"),
      [&](const std::string& body) {
        const nlohmann::json data = nlohmann::json::parse(body);
        if (!data.contains("start")) return false;
        if (data["start"]) {
          StartCalibration(kCalibrationMethod_Ui);
        } else {
          CompleteCalibration(kCalibrationMethod_Ui);
        }

        return true;
      });
};

vr::TrackedDevicePose_t DevicePose::GetControllerPose(uint32_t controller_id) {
  vr::TrackedDevicePose_t poses[vr::k_unMaxTrackedDeviceCount];
  vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, poses, vr::k_unMaxTrackedDeviceCount);
  return poses[controller_id];
}

vr::DriverPose_t DevicePose::UpdatePose() const {
  if (calibration_->IsCalibrating()) return calibration_->GetMaintainPose();

  vr::DriverPose_t result{};
  result.qDriverFromHeadRotation.w = 1;
  result.qWorldFromDriverRotation.w = 1;

  if (controller_id_ < 1) {
    result.poseIsValid = false;
    result.result = vr::TrackingResult_Uninitialized;

    return result;
  }

  const vr::TrackedDevicePose_t controller_pose = GetControllerPose(controller_id_);
  if (!controller_pose.bPoseIsValid) {
    result.result = vr::TrackingResult_Running_OK;
    result.poseIsValid = true;

    return result;
  }

  const vr::HmdVector3d_t controller_position = MatrixToPosition(controller_pose.mDeviceToAbsoluteTracking);
  const vr::HmdQuaternion_t controller_orientation = MatrixToOrientation(controller_pose.mDeviceToAbsoluteTracking);

  const vr::HmdQuaternion_t rotation = controller_orientation * configuration_.offset_orientation;
  result.qRotation = rotation;

  const vr::HmdVector3d_t position = controller_position + (configuration_.offset_position * controller_orientation);
  result.vecPosition[0] = position.v[0];
  result.vecPosition[1] = position.v[1];
  result.vecPosition[2] = position.v[2];

  const vr::HmdVector3_t velocity = controller_pose.vVelocity;
  result.vecVelocity[0] = velocity.v[0];
  result.vecVelocity[1] = velocity.v[1];
  result.vecVelocity[2] = velocity.v[2];

  result.poseIsValid = true;
  result.deviceIsConnected = true;

  result.result = vr::TrackingResult_Running_OK;

  return result;
}

void DevicePose::StartCalibration(CalibrationMethod method) const {
  calibration_->StartCalibration(UpdatePose(), method);
}

void DevicePose::CancelCalibration(CalibrationMethod method) const {
  calibration_->CancelCalibration(method);
}

bool DevicePose::IsCalibrating() const {
  return calibration_->IsCalibrating();
}

void DevicePose::CompleteCalibration(CalibrationMethod method) {
  if (controller_id_ < 0) {
    DriverLog("Completed calibration but controller index was invalid");
    CancelCalibration(method);
    return;
  }

  configuration_ =
      calibration_->CompleteCalibration(GetControllerPose(controller_id_), configuration_, role_ == vr::TrackedControllerRole_RightHand, method);

  SetPoseConfiguration(configuration_, role_);
}