#include "Calibration.h"

#include <queue>
#include <utility>

#include "DriverLog.h"
#include "Quaternion.h"

Calibration::Calibration() {}

void Calibration::StartCalibration(vr::DriverPose_t maintainPose) {
    m_maintainPose = maintainPose;  
    m_isCalibrating = true;
}

void Calibration::FinishCalibration(vr::TrackedDevicePose_t controllerPose, VRPoseConfiguration_t poseConfiguration, bool isRightHand) {
    
    m_isCalibrating = false;
    // get the matrix that represents the position of the controller that we are shadowing
    vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

    vr::HmdQuaternion_t controllerQuat = GetRotation(controllerMatrix);
    vr::HmdQuaternion_t handQuat = m_maintainPose.qRotation;
    

    //qC * qT = qH   -> qC*qC^-1 * qT = qH * qC^-1   -> qT = qH * qC^-1
    vr::HmdQuaternion_t transformQuat = MultiplyQuaternion(QuatConjugate(controllerQuat), handQuat);
    //m_poseConfiguration.angleOffsetQuaternion = transformQuat;
    poseConfiguration.angleOffsetQuaternion.w = transformQuat.w;
    poseConfiguration.angleOffsetQuaternion.x = transformQuat.x;
    poseConfiguration.angleOffsetQuaternion.y = transformQuat.y;
    poseConfiguration.angleOffsetQuaternion.z = transformQuat.z;

    vr::HmdVector3_t differenceVector = { m_maintainPose.vecPosition[0] - controllerMatrix.m[0][3],
                                          m_maintainPose.vecPosition[1] - controllerMatrix.m[1][3],
                                          m_maintainPose.vecPosition[2] - controllerMatrix.m[2][3] };

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


}

void Calibration::CancelCalibration() { m_isCalibrating = false; }

bool Calibration::isCalibrating() {
    return m_isCalibrating;
}

vr::DriverPose_t Calibration::GetMaintainPose() {
    m_maintainPose.vecVelocity[0] = 0;
    m_maintainPose.vecVelocity[1] = 0;
    m_maintainPose.vecVelocity[2] = 0;
    m_maintainPose.vecAngularVelocity[0] = 0;
    m_maintainPose.vecAngularVelocity[1] = 0;
    m_maintainPose.vecAngularVelocity[2] = 0;
    return m_maintainPose;
}

