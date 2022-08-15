#include "device_pose.h"

#include "external_services/driver_internal.h"
#include "util/driver_log.h"
#include "util/driver_math.h"

static DriverInternalServer& driver_server = DriverInternalServer::GetInstance();

DevicePose::DevicePose(vr::ETrackedControllerRole role) : role_(role), calibration_(std::make_unique<PoseCalibration>()) {
  configuration_ = GetPoseConfiguration(role);

  driver_server.AddTrackingReferenceRequestCallback([&](const TrackingReferenceResult& result) {
    if (result.role == role_) {
      DriverLog(
          "Controller that %s hand is tracking from has been updated to id: %i",
          role_ == vr::TrackedControllerRole_RightHand ? "right" : "left",
          result.controller_id);
      controller_id_ = result.controller_id;
    }
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

  const vr::TrackedDevicePose_t controllerPose = GetControllerPose(controller_id_);
  if (!controllerPose.bPoseIsValid) {
    result.poseIsValid = false;
    result.result = vr::TrackingResult_Uninitialized;

    return result;
  }

  const vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

  const vr::HmdQuaternion_t controllerRotation = MatrixToOrientation(controllerMatrix);
  const vr::HmdVector3d_t controllerPosition = MatrixToPosition(controllerMatrix);

  result.qWorldFromDriverRotation = controllerRotation;

  result.vecWorldFromDriverTranslation[0] = controllerPosition.v[0];
  result.vecWorldFromDriverTranslation[1] = controllerPosition.v[1];
  result.vecWorldFromDriverTranslation[2] = controllerPosition.v[2];

  result.vecPosition[0] = configuration_.offset_position.v[0];
  result.vecPosition[1] = configuration_.offset_position.v[1];
  result.vecPosition[2] = configuration_.offset_position.v[2];

  const vr::HmdVector3_t objectVelocity = controllerPose.vVelocity * -controllerRotation;
  result.vecVelocity[0] = objectVelocity.v[0];
  result.vecVelocity[1] = objectVelocity.v[1];
  result.vecVelocity[2] = objectVelocity.v[2];

  const vr::HmdVector3_t objectAngularVelocity = controllerPose.vAngularVelocity * -controllerRotation * -configuration_.offset_orientation;
  result.vecAngularVelocity[0] = objectAngularVelocity.v[0];
  result.vecAngularVelocity[1] = objectAngularVelocity.v[1];
  result.vecAngularVelocity[2] = objectAngularVelocity.v[2];

  result.qRotation = configuration_.offset_orientation;

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