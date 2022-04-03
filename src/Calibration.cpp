#include "Calibration.h"

#include "DriverLog.h"
#include "Util/Quaternion.h"

Calibration::Calibration() : maintainPose_(), isCalibrating_(false), calibratingMethod_(CalibrationMethod::None) {}

void Calibration::StartCalibration(vr::DriverPose_t maintainPose, const CalibrationMethod method) {
  calibratingMethod_ = method;

  maintainPose.vecVelocity[0] = 0;
  maintainPose.vecVelocity[1] = 0;
  maintainPose.vecVelocity[2] = 0;
  maintainPose.vecAngularVelocity[0] = 0;
  maintainPose.vecAngularVelocity[1] = 0;
  maintainPose.vecAngularVelocity[2] = 0;
  maintainPose_ = maintainPose;
  isCalibrating_ = true;
}

VRPoseConfiguration Calibration::CompleteCalibration(
    const vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration poseConfiguration, const bool isRightHand, const CalibrationMethod method) {
  if (calibratingMethod_ != method) return poseConfiguration;

  isCalibrating_ = false;

  // controllerPose = new position of controller
  // controllerPose contains the position of the controller as qRotation and vecPosition

  // maintainPose = previous position of controller
  // maintainPose contains the position of the controller as DriverFromWorld<Rotation/Translation> and the offsets we apply as qRotation and
  // vecPosition

  const vr::HmdVector3d_t newControllerPosition = GetPosition(controllerPose.mDeviceToAbsoluteTracking);
  const vr::HmdQuaternion_t newControllerRotation = GetRotation(controllerPose.mDeviceToAbsoluteTracking);

  const vr::HmdVector3d_t lastControllerPosition{
      maintainPose_.vecWorldFromDriverTranslation[0] + poseConfiguration.offsetVector.v[0],
      maintainPose_.vecWorldFromDriverTranslation[1] + poseConfiguration.offsetVector.v[1],
      maintainPose_.vecWorldFromDriverTranslation[2] + poseConfiguration.offsetVector.v[2],
  };
  const vr::HmdQuaternion_t lastControllerRotation = maintainPose_.qWorldFromDriverRotation * poseConfiguration.angleOffsetQuaternion;

  const vr::HmdQuaternion_t transformQuat = -newControllerRotation * lastControllerRotation;
  poseConfiguration.angleOffsetQuaternion = transformQuat;

  const vr::HmdVector3d_t differenceVector = lastControllerPosition - newControllerPosition;
  const vr::HmdVector3d_t transformVector = differenceVector * -newControllerRotation;

  poseConfiguration.offsetVector = transformVector;

  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position", transformVector.v[0]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position", transformVector.v[1]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position", transformVector.v[2]);

  const vr::HmdVector3d_t eulerOffset = QuaternionToEuler(transformQuat);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees", RadToDeg(eulerOffset.v[2]));
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees", RadToDeg(eulerOffset.v[1]));
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees", RadToDeg(eulerOffset.v[0]));

  return poseConfiguration;
}

void Calibration::CancelCalibration(CalibrationMethod method) {
  if (calibratingMethod_ == method) isCalibrating_ = false;
}

bool Calibration::IsCalibrating() const {
  return isCalibrating_;
}

vr::DriverPose_t Calibration::GetMaintainPose() const {
  return maintainPose_;
}
