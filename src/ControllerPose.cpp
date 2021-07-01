#include "ControllerPose.h"

#include <queue>
#include <utility>

#include "DriverLog.h"
#include "Quaternion.h"

ControllerPose::ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole,
                               std::string thisDeviceManufacturer,
                               VRPoseConfiguration_t poseConfiguration)
    : m_shadowDeviceOfRole(shadowDeviceOfRole),
      m_thisDeviceManufacturer(std::move(thisDeviceManufacturer)),
      m_poseConfiguration(poseConfiguration) {


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

vr::DriverPose_t ControllerPose::UpdatePose() {
  if (m_isCalibrating) return m_maintainPose;
  vr::DriverPose_t newPose = {0};
  newPose.qWorldFromDriverRotation.w = 1;
  newPose.qDriverFromHeadRotation.w = 1;

  if (m_shadowControllerId != vr::k_unTrackedDeviceIndexInvalid) {
    vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
    vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses,
                                                       vr::k_unMaxTrackedDeviceCount);

    if (trackedDevicePoses[m_shadowControllerId].bPoseIsValid) {
      // get the matrix that represents the position of the controller that we are shadowing
      vr::HmdMatrix34_t controllerMatrix =
          trackedDevicePoses[m_shadowControllerId].mDeviceToAbsoluteTracking;

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
      newPose.vecAngularVelocity[0] =
          trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[0];
      newPose.vecAngularVelocity[1] =
          trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[1];
      newPose.vecAngularVelocity[2] =
          trackedDevicePoses[m_shadowControllerId].vAngularVelocity.v[2];

      newPose.vecVelocity[0] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[0];
      newPose.vecVelocity[1] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[1];
      newPose.vecVelocity[2] = trackedDevicePoses[m_shadowControllerId].vVelocity.v[2];

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
    m_isCalibrating = true;
    m_maintainPose = UpdatePose();
}

void ControllerPose::FinishCalibration() {
    m_isCalibrating = false;
    vr::DriverPose_t finishPose = UpdatePose();
    //add logic for calculating and updating pose settings
    //vr::VRSettings()->SetInt32()..., logic may need to move to the DeviceProvider
}

void ControllerPose::CancelCalibration() { m_isCalibrating = false; }