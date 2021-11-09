#include "Calibration.h"

#include "Quaternion.h"

Calibration::Calibration() : _maintainPose(), _isCalibrating(false), _calibratingMethod(CalibrationMethod::None) {}

void Calibration::StartCalibration(vr::DriverPose_t maintainPose, const CalibrationMethod method) {
  _calibratingMethod = method;

  maintainPose.vecVelocity[0] = 0;
  maintainPose.vecVelocity[1] = 0;
  maintainPose.vecVelocity[2] = 0;
  maintainPose.vecAngularVelocity[0] = 0;
  maintainPose.vecAngularVelocity[1] = 0;
  maintainPose.vecAngularVelocity[2] = 0;
  _maintainPose = maintainPose;
  _isCalibrating = true;
}

VRPoseConfiguration Calibration::CompleteCalibration(
    const vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration poseConfiguration, const bool isRightHand, const CalibrationMethod method) {
  if (_calibratingMethod != method) return poseConfiguration;

  _isCalibrating = false;
  // get the matrix that represents the position of the controller that we are shadowing
  const vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

  const vr::HmdQuaternion_t controllerQuat = GetRotation(controllerMatrix);
  const vr::HmdQuaternion_t handQuat = _maintainPose.qRotation;

  // qC * qT = qH   -> qC*qC^-1 * qT = qH * qC^-1   -> qT = qH * qC^-1
  const vr::HmdQuaternion_t transformQuat = MultiplyQuaternion(QuatConjugate(controllerQuat), handQuat);

  poseConfiguration.angleOffsetQuaternion.w = transformQuat.w;
  poseConfiguration.angleOffsetQuaternion.x = transformQuat.x;
  poseConfiguration.angleOffsetQuaternion.y = transformQuat.y;
  poseConfiguration.angleOffsetQuaternion.z = transformQuat.z;

  const vr::HmdVector3_t differenceVector = {
      static_cast<float>(_maintainPose.vecPosition[0] - controllerMatrix.m[0][3]),
      static_cast<float>(_maintainPose.vecPosition[1] - controllerMatrix.m[1][3]),
      static_cast<float>(_maintainPose.vecPosition[2] - controllerMatrix.m[2][3])};

  const vr::HmdQuaternion_t transformInverse = QuatConjugate(controllerQuat);
  const vr::HmdMatrix33_t transformMatrix = QuaternionToMatrix(transformInverse);
  const vr::HmdVector3_t transformVector = MultiplyMatrix(transformMatrix, differenceVector);

  poseConfiguration.offsetVector = transformVector;

  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position", transformVector.v[0]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position", transformVector.v[1]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position", transformVector.v[2]);

  const vr::HmdVector3_t eulerOffset = QuaternionToEuler(transformQuat);

  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees", eulerOffset.v[0]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees", eulerOffset.v[1]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees", eulerOffset.v[2]);

  return poseConfiguration;
}

void Calibration::CancelCalibration(CalibrationMethod method) {
  if (_calibratingMethod == method) _isCalibrating = false;
}

bool Calibration::IsCalibrating() const {
  return _isCalibrating;
}

vr::DriverPose_t Calibration::GetMaintainPose() const {
  return _maintainPose;
}
