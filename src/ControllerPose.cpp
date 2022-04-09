#include "ControllerPose.h"

#include "DriverLog.h"
#include "Util/Quaternion.h"

ControllerPose::ControllerPose(
    vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRPoseConfiguration poseConfiguration)
    : poseConfiguration_(poseConfiguration), shadowDeviceOfRole_(shadowDeviceOfRole), thisDeviceManufacturer_(std::move(thisDeviceManufacturer)) {
  calibrationPipe_ = std::make_unique<NamedPipeListener<CalibrationDataIn>>(
      R"(\\.\pipe\vrapplication\functions\autocalibrate\)" +
          std::string(shadowDeviceOfRole == vr::ETrackedControllerRole::TrackedControllerRole_RightHand ? "right" : "left"),
      [&](const CalibrationDataIn* data) {
        if (data->start) {
          DriverLog("Starting calibration via external application");
          StartCalibration(CalibrationMethod::Ui);
        } else {
          DriverLog("Stopping calibration via external application");
          CompleteCalibration(CalibrationMethod::Ui);
        }
      });

  calibrationPipe_->StartListening();

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
  if (controllerDiscoverer_ != nullptr) controllerDiscoverer_->Stop();
}

vr::TrackedDevicePose_t ControllerPose::GetControllerPose() const {
  vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
  vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0, trackedDevicePoses, vr::k_unMaxTrackedDeviceCount);
  return trackedDevicePoses[shadowControllerId_];
}

vr::DriverPose_t ControllerPose::UpdatePose() const {
  if (calibration_->IsCalibrating()) return calibration_->GetMaintainPose();

  vr::DriverPose_t newPose = {0};
  newPose.qDriverFromHeadRotation.w = 1;
  newPose.qWorldFromDriverRotation.w = 1;

  if (shadowControllerId_ != vr::k_unTrackedDeviceIndexInvalid) {
    const vr::TrackedDevicePose_t controllerPose = GetControllerPose();
    if (controllerPose.bPoseIsValid) {
      const vr::HmdMatrix34_t controllerMatrix = controllerPose.mDeviceToAbsoluteTracking;

      const vr::HmdQuaternion_t controllerRotation = GetRotation(controllerMatrix);
      const vr::HmdVector3d_t controllerPosition = GetPosition(controllerMatrix);

      newPose.qWorldFromDriverRotation = controllerRotation;

      newPose.vecWorldFromDriverTranslation[0] = controllerPosition.v[0];
      newPose.vecWorldFromDriverTranslation[1] = controllerPosition.v[1];
      newPose.vecWorldFromDriverTranslation[2] = controllerPosition.v[2];

      newPose.vecPosition[0] = poseConfiguration_.offsetVector.v[0];
      newPose.vecPosition[1] = poseConfiguration_.offsetVector.v[1];
      newPose.vecPosition[2] = poseConfiguration_.offsetVector.v[2];

      const vr::HmdVector3_t objectVelocity = controllerPose.vVelocity * -controllerRotation;

      newPose.vecVelocity[0] = objectVelocity.v[0];
      newPose.vecVelocity[1] = objectVelocity.v[1];
      newPose.vecVelocity[2] = objectVelocity.v[2];

      const vr::HmdVector3_t objectAngularVelocity = controllerPose.vAngularVelocity * -controllerRotation * -poseConfiguration_.angleOffsetQuaternion;

      newPose.vecAngularVelocity[0] = objectAngularVelocity.v[0];
      newPose.vecAngularVelocity[1] = objectAngularVelocity.v[1];
      newPose.vecAngularVelocity[2] = objectAngularVelocity.v[2];

      newPose.qRotation = poseConfiguration_.angleOffsetQuaternion;

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