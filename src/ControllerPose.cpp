#include "ControllerPose.h"

#include "DriverLog.h"
#include "Util/Quaternion.h"

#define _USE_MATH_DEFINES
#include <math.h>

ControllerPose::ControllerPose(
    vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRPoseConfiguration poseConfiguration)
    : poseConfiguration_(poseConfiguration), shadowDeviceOfRole_(shadowDeviceOfRole), thisDeviceManufacturer_(std::move(thisDeviceManufacturer)) {
  calibrationPipe_ = std::make_unique<NamedPipeListener<CalibrationDataIn>>(
      R"(\\.\pipe\vrapplication\functions\autocalibrate\)" +
      std::string(shadowDeviceOfRole == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left"));

  calibrationPipe_->StartListening([&](const CalibrationDataIn* data) {
    if (data->start) {
      DriverLog("Starting calibration via external application");
      StartCalibration(CalibrationMethod::Ui);
    } else {
      DriverLog("Stopping calibration via external application");
      CompleteCalibration(CalibrationMethod::Ui);
    }
  });

  if (poseConfiguration_.controllerOverrideEnabled) {
    shadowControllerId_ = poseConfiguration_.controllerIdOverride;
  } else {
    controllerDiscoverer_ = std::make_unique<ControllerDiscovery>(shadowDeviceOfRole, [&](const ControllerDiscoveryPipeData data) {
      shadowControllerId_ = data.controllerId;
      DriverLog("Received controller id from overlay: %i", data.controllerId);
    });

    controllerDiscoverer_->Start();
  }
  calibration_ = std::make_unique<Calibration>();
}

ControllerPose::~ControllerPose() {
  calibrationPipe_->StopListening();
  if (controllerDiscoverer_) controllerDiscoverer_->Stop();
}

vr::TrackedDevicePose_t ControllerPose::GetControllerPose() const {
  vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
  vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses, vr::k_unMaxTrackedDeviceCount);
  return trackedDevicePoses[shadowControllerId_];
}

vr::DriverPose_t ControllerPose::UpdatePose() const {
  if (calibration_->IsCalibrating()) return calibration_->GetMaintainPose();

  vr::DriverPose_t newPose = {0};
  newPose.qWorldFromDriverRotation.w = 1;
  newPose.qDriverFromHeadRotation.w = 1;

  if (shadowControllerId_ != vr::k_unTrackedDeviceIndexInvalid) {
    const vr::TrackedDevicePose_t controllerPose = GetControllerPose();
    if (controllerPose.bPoseIsValid) {
      // get the matrix that represents the position of the controller that we are shadowing
      const vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

      // get only the rotation (3x3 matrix), as the 3x4 matrix also includes position
      const vr::HmdMatrix33_t controllerRotationMatrix = GetRotationMatrix(controllerMatrix);
      const vr::HmdQuaternion_t controllerRotation = GetRotation(controllerMatrix);

      const vr::HmdVector3_t vectorOffset = MultiplyMatrix(controllerRotationMatrix, poseConfiguration_.offsetVector);

      // combine these positions to get the resultant position
      const vr::HmdVector3_t newControllerPosition = CombinePosition(controllerMatrix, vectorOffset);

      newPose.vecPosition[0] = newControllerPosition.v[0];
      newPose.vecPosition[1] = newControllerPosition.v[1];
      newPose.vecPosition[2] = newControllerPosition.v[2];

      newPose.qRotation = MultiplyQuaternion(controllerRotation, poseConfiguration_.angleOffsetQuaternion);

      const vr::HmdVector3_t controllerAngularVelocity = controllerPose.vAngularVelocity;
      const vr::HmdQuaternion_t qControllerAngularVelocity = {
          0.0, controllerAngularVelocity.v[0], controllerAngularVelocity.v[1], controllerAngularVelocity.v[2]};

      const vr::HmdQuaternion_t result =
          MultiplyQuaternion(MultiplyQuaternion(newPose.qRotation, qControllerAngularVelocity), QuatConjugate(newPose.qRotation));

      newPose.vecAngularVelocity[0] = result.x;
      newPose.vecAngularVelocity[1] = result.y;
      newPose.vecAngularVelocity[2] = result.z;

      newPose.vecVelocity[0] = controllerPose.vVelocity.v[0];
      newPose.vecVelocity[1] = controllerPose.vVelocity.v[1];
      newPose.vecVelocity[2] = controllerPose.vVelocity.v[2];

      newPose.poseTimeOffset = poseConfiguration_.poseTimeOffset;
      newPose.poseIsValid = true;
      newPose.deviceIsConnected = true;

      newPose.result = vr::TrackingResult_Running_OK;
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

void ControllerPose::StartCalibration(const CalibrationMethod method) const {
  calibration_->StartCalibration(UpdatePose(), method);
}

void ControllerPose::CompleteCalibration(const CalibrationMethod method) {
  if (shadowControllerId_ == vr::k_unTrackedDeviceIndexInvalid) {
    DriverLog("Index invalid");
    CancelCalibration(method);
    return;
  }
  poseConfiguration_ = calibration_->CompleteCalibration(GetControllerPose(), poseConfiguration_, IsRightHand(), method);
}

void ControllerPose::CancelCalibration(const CalibrationMethod method) const {
  calibration_->CancelCalibration(method);
}

bool ControllerPose::IsCalibrating() const {
  return calibration_->IsCalibrating();
}

bool ControllerPose::IsRightHand() const {
  return shadowDeviceOfRole_ == vr::TrackedControllerRole_RightHand;
}
