#include "ControllerPose.h"

#include <queue>
#include <utility>

#include "DriverLog.h"
#include "Quaternion.h"

ControllerPose::ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole,
                               std::string thisDeviceManufacturer,
                               VRPoseConfiguration_t poseConfiguration, bool isRightHand)
    : m_shadowDeviceOfRole(shadowDeviceOfRole),
      m_thisDeviceManufacturer(std::move(thisDeviceManufacturer)),
      m_poseConfiguration(poseConfiguration), m_isRightHand(isRightHand) {


  if (m_poseConfiguration.controllerOverrideEnabled) {
    m_shadowControllerId = m_poseConfiguration.controllerIdOverride;
  } else {
    m_controllerDiscoverer = std::make_unique<ControllerDiscoveryPipe>();

    m_controllerDiscoverer->Start(
        [&](ControllerPipeData data) { 
            m_shadowControllerId = data.controllerId;
          DebugDriverLog("Received message! %i", data.controllerId);
        },
        m_shadowDeviceOfRole);
  }
  
}

vr::TrackedDevicePose_t ControllerPose::GetControllerPose() {
    vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses,
        vr::k_unMaxTrackedDeviceCount);
    return trackedDevicePoses[m_shadowControllerId];
}

vr::DriverPose_t ControllerPose::UpdatePose() {
    if (m_isCalibrating) {
        m_maintainPose.vecVelocity[0] = 0;
        m_maintainPose.vecVelocity[1] = 0;
        m_maintainPose.vecVelocity[2] = 0;
        m_maintainPose.vecAngularVelocity[0] = 0;
        m_maintainPose.vecAngularVelocity[1] = 0;
        m_maintainPose.vecAngularVelocity[2] = 0;
        return m_maintainPose;
    }
  vr::DriverPose_t newPose = {0};
  newPose.qWorldFromDriverRotation.w = 1;
  newPose.qDriverFromHeadRotation.w = 1;

  if (m_shadowControllerId != vr::k_unTrackedDeviceIndexInvalid) {
      vr::TrackedDevicePose_t controllerPose = GetControllerPose();

    if (controllerPose.bPoseIsValid) {
      // get the matrix that represents the position of the controller that we are shadowing
      vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

      // get only the rotation (3x3 matrix), as the 3x4 matrix also includes position
      vr::HmdMatrix33_t controllerRotationMatrix = GetRotationMatrix(controllerMatrix);

      // multiply the rotation matrix by the offset vector set that is the offset of the controller
      // relative to the hand
      vr::HmdVector3_t vectorOffset =
          MultiplyMatrix(controllerRotationMatrix, m_poseConfiguration.offsetVector);

      // combine these positions to get the resultant position
      vr::HmdVector3_t newControllerPosition = CombinePosition(controllerMatrix, vectorOffset);

      newPose.vecPosition[0] = newControllerPosition.v[0];
      newPose.vecPosition[1] = newControllerPosition.v[1];
      newPose.vecPosition[2] = newControllerPosition.v[2];

      // Multiply rotation quaternions together, as the controller may be rotated relative to the
      // hand
      newPose.qRotation = MultiplyQuaternion(GetRotation(controllerMatrix),
                                             m_poseConfiguration.angleOffsetQuaternion);

      // Copy other values from the controller that we want for this device
      newPose.vecAngularVelocity[0] = controllerPose.vAngularVelocity.v[0];
      newPose.vecAngularVelocity[1] = controllerPose.vAngularVelocity.v[1];
      newPose.vecAngularVelocity[2] = controllerPose.vAngularVelocity.v[2];

      newPose.vecVelocity[0] = controllerPose.vVelocity.v[0];
      newPose.vecVelocity[1] = controllerPose.vVelocity.v[1];
      newPose.vecVelocity[2] = controllerPose.vVelocity.v[2];

      newPose.poseIsValid = true;
      newPose.deviceIsConnected = true;

      newPose.result = vr::TrackingResult_Running_OK;

      newPose.poseTimeOffset = m_poseConfiguration.poseOffset;
    } else {
      newPose.poseIsValid = false;
      newPose.deviceIsConnected = true;
      newPose.result = vr::TrackingResult_Uninitialized;
    }

  } else {
    newPose.result = vr::TrackingResult_Uninitialized;
    newPose.deviceIsConnected = false;
  }

  return newPose;
}

void ControllerPose::StartCalibration() {
    m_maintainPose = UpdatePose();  
    m_isCalibrating = true;
}

void ControllerPose::FinishCalibration() {
    m_isCalibrating = false;
    vr::TrackedDevicePose_t controllerPose = GetControllerPose();
    // get the matrix that represents the position of the controller that we are shadowing
    vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

    //add logic for calculating and updating pose settings
    //vr::VRSettings()->SetInt32()..., logic may need to move to the DeviceProvider


    vr::HmdQuaternion_t controllerQuat = GetRotation(controllerMatrix);
    vr::HmdQuaternion_t handQuat = m_maintainPose.qRotation;
    

    //qC * qT = qH   -> qC*qC^-1 * qT = qH * qC^-1   -> qT = qH * qC^-1
    vr::HmdQuaternion_t transformQuat = MultiplyQuaternion(QuatConjugate(controllerQuat), handQuat);
    //m_poseConfiguration.angleOffsetQuaternion = transformQuat;
    {
        m_poseConfiguration.angleOffsetQuaternion.w = transformQuat.w;
        m_poseConfiguration.angleOffsetQuaternion.x = transformQuat.x;
        m_poseConfiguration.angleOffsetQuaternion.y = transformQuat.y;
        m_poseConfiguration.angleOffsetQuaternion.z = transformQuat.z;
    }
    
    float contNorm = QuatNorm(controllerQuat);
    float handNorm = QuatNorm(handQuat);
    float offsetNorm = QuatNorm(m_poseConfiguration.angleOffsetQuaternion);
    if (contNorm) {
        DebugDriverLog("Non unit quat from controller!");
    }
    if (handNorm) {
        DebugDriverLog("Non unit quat from hand!");
    }
    if (offsetNorm) {
        DebugDriverLog("Non unit quat from offset!");
    }

    vr::HmdVector3_t differenceVector = { m_maintainPose.vecPosition[0] - controllerMatrix.m[0][3],
                                          m_maintainPose.vecPosition[1] - controllerMatrix.m[1][3],
                                          m_maintainPose.vecPosition[2] - controllerMatrix.m[2][3] };

    vr::HmdQuaternion_t transformInverse = QuatConjugate(controllerQuat);
    vr::HmdMatrix33_t transformMatrix = QuaternionToMatrix(transformInverse);
    vr::HmdVector3_t transformVector = MultiplyMatrix(transformMatrix, differenceVector);

    m_poseConfiguration.offsetVector = transformVector;

    vr::VRSettings()->SetFloat(c_poseSettingsSection, m_isRightHand ? "right_x_offset_position" : "left_x_offset_position", transformVector.v[0]);
    vr::VRSettings()->SetFloat(c_poseSettingsSection, m_isRightHand ? "right_y_offset_position" : "left_y_offset_position", transformVector.v[1]);
    vr::VRSettings()->SetFloat(c_poseSettingsSection, m_isRightHand ? "right_z_offset_position" : "left_z_offset_position", transformVector.v[2]);

    vr::HmdVector3_t eulerOffset = QuaternionToEuler(transformQuat);

    vr::VRSettings()->SetFloat(c_poseSettingsSection, m_isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees", eulerOffset.v[0]);
    vr::VRSettings()->SetFloat(c_poseSettingsSection, m_isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees", eulerOffset.v[1]);
    vr::VRSettings()->SetFloat(c_poseSettingsSection, m_isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees", eulerOffset.v[2]);


}

void ControllerPose::CancelCalibration() { m_isCalibrating = false; }

bool ControllerPose::isCalibrating() {
    return m_isCalibrating;
}