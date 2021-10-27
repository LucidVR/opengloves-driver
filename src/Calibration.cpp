#include "Calibration.h"

#include "DriverLog.h"
#include "Quaternion.h"

Calibration::Calibration() : m_maintainPose(), m_isCalibrating(false), m_calibratingMethod(CalibrationMethod::NONE) {}

void Calibration::StartCalibration(vr::DriverPose_t maintainPose, CalibrationMethod method) {
  m_calibratingMethod = method;

  maintainPose.vecVelocity[0] = 0;
  maintainPose.vecVelocity[1] = 0;
  maintainPose.vecVelocity[2] = 0;
  maintainPose.vecAngularVelocity[0] = 0;
  maintainPose.vecAngularVelocity[1] = 0;
  maintainPose.vecAngularVelocity[2] = 0;
  m_maintainPose = maintainPose;
  m_isCalibrating = true;
}

VRPoseConfiguration Calibration::CompleteCalibration(vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration poseConfiguration, bool isRightHand,
                                                       CalibrationMethod method) {
  if (m_calibratingMethod != method) return poseConfiguration;
  m_isCalibrating = false;
  // get the matrix that represents the position of the controller that we are shadowing
  vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

  vr::HmdQuaternion_t controllerQuat = GetRotation(controllerMatrix);
  vr::HmdQuaternion_t handQuat = m_maintainPose.qRotation;

  // qC * qT = qH   -> qC*qC^-1 * qT = qH * qC^-1   -> qT = qH * qC^-1
  vr::HmdQuaternion_t transformQuat = MultiplyQuaternion(QuatConjugate(controllerQuat), handQuat);

  poseConfiguration.angleOffsetQuaternion.w = transformQuat.w;
  poseConfiguration.angleOffsetQuaternion.x = transformQuat.x;
  poseConfiguration.angleOffsetQuaternion.y = transformQuat.y;
  poseConfiguration.angleOffsetQuaternion.z = transformQuat.z;

  vr::HmdVector3_t differenceVector = {(float)(m_maintainPose.vecPosition[0] - controllerMatrix.m[0][3]),
                                       (float)(m_maintainPose.vecPosition[1] - controllerMatrix.m[1][3]),
                                       (float)(m_maintainPose.vecPosition[2] - controllerMatrix.m[2][3])};

  vr::HmdQuaternion_t transformInverse = QuatConjugate(controllerQuat);
  vr::HmdMatrix33_t transformMatrix = QuaternionToMatrix(transformInverse);
  vr::HmdVector3_t transformVector = MultiplyMatrix(transformMatrix, differenceVector);

  poseConfiguration.offsetVector = transformVector;

  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position", transformVector.v[0]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position", transformVector.v[1]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position", transformVector.v[2]);

  vr::HmdVector3_t eulerOffset = QuaternionToEuler(transformQuat);

  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees", eulerOffset.v[0]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees", eulerOffset.v[1]);
  vr::VRSettings()->SetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees", eulerOffset.v[2]);

  return poseConfiguration;
}

void Calibration::CancelCalibration(CalibrationMethod method) {
  if (m_calibratingMethod == method) m_isCalibrating = false;
}

bool Calibration::isCalibrating() { return m_isCalibrating; }

vr::DriverPose_t Calibration::GetMaintainPose() { return m_maintainPose; }
