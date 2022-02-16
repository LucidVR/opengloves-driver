#include "DeviceDriver/PhysicalDeviceDriver/PhysicalDeviceDriver.h"

PhysicalDeviceDriver::PhysicalDeviceDriver(
    std::unique_ptr<CommunicationManager> communicationManager,
    std::unique_ptr<BoneAnimator> boneAnimator,
    std::string serialNumber,
    VRDeviceConfiguration configuration)
    : DeviceDriver(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration){};

void PhysicalDeviceDriver::StartingDevice() {
  controllerPose_ = std::make_unique<ControllerPose>(configuration_.role, std::string(c_deviceDriverManufacturer), configuration_.poseConfiguration);

  poseUpdateThread_ = std::thread(&PhysicalDeviceDriver::PoseUpdateThread, this);
}

void PhysicalDeviceDriver::HandleInput(const VRInputData& data) {
  if (configuration_.poseConfiguration.calibrationButtonEnabled) {
    if (data.calibrate) {
      if (!controllerPose_->IsCalibrating()) controllerPose_->StartCalibration(CalibrationMethod::Hardware);
    } else {
      if (controllerPose_->IsCalibrating()) controllerPose_->CompleteCalibration(CalibrationMethod::Hardware);
    }
  }
}

void PhysicalDeviceDriver::PoseUpdateThread() {
  while (IsActive()) {
    vr::DriverPose_t pose = controllerPose_->UpdatePose();
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(driverId_, pose, sizeof(vr::DriverPose_t));

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  DriverLog("Closing pose thread...");
}

vr::DriverPose_t PhysicalDeviceDriver::GetPose() {
  if (IsActive()) return controllerPose_->UpdatePose();

  return vr::DriverPose_t{0};
}

void PhysicalDeviceDriver::StoppingDevice() {
  if (poseUpdateThread_.joinable()) poseUpdateThread_.join();
}