#include "ControllerPose.h"

#include "DriverLog.h"
#include "Quaternion.h"

ControllerPose::ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRPoseConfiguration poseConfiguration)
    : m_shadowDeviceOfRole(shadowDeviceOfRole), m_thisDeviceManufacturer(std::move(thisDeviceManufacturer)), m_poseConfiguration(poseConfiguration) {
  m_calibrationPipe = std::make_unique<NamedPipeListener<CalibrationDataIn>>(
      "\\\\.\\pipe\\vrapplication\\functions\\autocalibrate\\" +
      std::string(shadowDeviceOfRole == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left"));

  m_calibrationPipe->StartListening([&](CalibrationDataIn* data) {
    if (data->start) {
      DriverLog("Starting calibration via external application");
      StartCalibration(CalibrationMethod::UI);
    } else {
      DriverLog("Stopping calibration via external application");
      CompleteCalibration(CalibrationMethod::UI);
    }
  });

  if (m_poseConfiguration.controllerOverrideEnabled) {
    m_shadowControllerId = m_poseConfiguration.controllerIdOverride;
  } else {
    m_controllerDiscoverer = std::make_unique<ControllerDiscovery>(shadowDeviceOfRole, [&](ControllerDiscoveryPipeData_t data) {
      m_shadowControllerId = data.controllerId;
      DriverLog("Received controller id from overlay: %i", data.controllerId);
    });

    m_controllerDiscoverer->Start();
  }
  m_calibration = std::make_unique<Calibration>();
}

ControllerPose::~ControllerPose() {
  m_calibrationPipe->StopListening();
  if (m_controllerDiscoverer) m_controllerDiscoverer->Stop();
}

vr::TrackedDevicePose_t ControllerPose::GetControllerPose() {
  vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
  vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses, vr::k_unMaxTrackedDeviceCount);
  return trackedDevicePoses[m_shadowControllerId];
}

vr::DriverPose_t ControllerPose::UpdatePose() {
  if (m_calibration->isCalibrating()) return m_calibration->GetMaintainPose();

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
      vr::HmdVector3_t vectorOffset = MultiplyMatrix(controllerRotationMatrix, m_poseConfiguration.offsetVector);

      // combine these positions to get the resultant position
      vr::HmdVector3_t newControllerPosition = CombinePosition(controllerMatrix, vectorOffset);

      newPose.vecPosition[0] = newControllerPosition.v[0];
      newPose.vecPosition[1] = newControllerPosition.v[1];
      newPose.vecPosition[2] = newControllerPosition.v[2];

      // Multiply rotation quaternions together, as the controller may be rotated relative to the
      // hand
      newPose.qRotation = MultiplyQuaternion(GetRotation(controllerMatrix), m_poseConfiguration.angleOffsetQuaternion);

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

      newPose.poseTimeOffset = m_poseConfiguration.poseTimeOffset;
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

void ControllerPose::StartCalibration(CalibrationMethod method) { m_calibration->StartCalibration(UpdatePose(), method); }

void ControllerPose::CompleteCalibration(CalibrationMethod method) {
  if (m_shadowControllerId == vr::k_unTrackedDeviceIndexInvalid) {
    DriverLog("Index invalid");
    CancelCalibration(method);
    return;
  }
  m_poseConfiguration = m_calibration->CompleteCalibration(GetControllerPose(), m_poseConfiguration, isRightHand(), method);
}

void ControllerPose::CancelCalibration(CalibrationMethod method) { m_calibration->CancelCalibration(method); }

bool ControllerPose::isCalibrating() { return m_calibration->isCalibrating(); }

bool ControllerPose::isRightHand() { return m_shadowDeviceOfRole == vr::TrackedControllerRole_RightHand; }