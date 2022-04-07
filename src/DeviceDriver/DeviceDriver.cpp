#include "DeviceDriver/DeviceDriver.h"

#include <utility>

#include "DriverLog.h"

DeviceDriver::DeviceDriver(
    std::unique_ptr<CommunicationManager> communicationManager, std::unique_ptr<BoneAnimator> boneAnimator, VRDeviceConfiguration configuration)
    : communicationManager_(std::move(communicationManager)),
      boneAnimator_(std::move(boneAnimator)),
      configuration_(std::move(configuration)),
      skeletalComponentHandle_(),
      handTransforms_(),
      hasActivated_(false),
      deviceId_(vr::k_unTrackedDeviceIndexInvalid) {
  // Load in a default skeleton
  boneAnimator_->LoadDefaultSkeletonByHand(handTransforms_, configuration_.role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
}

vr::EVRInitError DeviceDriver::Activate(uint32_t unObjectId) {
  deviceId_ = unObjectId;
  controllerPose_ = std::make_unique<ControllerPose>(configuration_.role, std::string(c_deviceManufacturer), configuration_.poseConfiguration);

  vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(
      deviceId_);  // this gets a container object where you store all the information about your driver

  SetupProps(props);

  vr::VRDriverInput()->CreateHapticComponent(props, "/output/haptic", &haptic_);
  vr::VRDriverInput()->CreateSkeletonComponent(
      props,
      IsRightHand() ? "/input/skeleton/right" : "/input/skeleton/left",
      IsRightHand() ? "/skeleton/hand/right" : "/skeleton/hand/left",
      "/pose/raw",
      vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Full,
      handTransforms_,
      NUM_BONES,
      &skeletalComponentHandle_);

  ffbProvider_ = std::make_unique<FFBListener>(
      [&](const VRFFBData data) {
        // Queue the force feedback data for sending.
        communicationManager_->QueueSend(data);
      },
      configuration_.role);

  ffbProvider_->Start();

  StartDevice();

  hasActivated_ = true;

  return vr::VRInitError_None;
}

void DeviceDriver::Deactivate() {
  if (hasActivated_.exchange(false)) {
    StoppingDevice();

    ffbProvider_->Stop();

    communicationManager_->Disconnect();
    deviceId_ = vr::k_unTrackedDeviceIndexInvalid;
    hasActivated_ = false;

    poseUpdateThread_.join();
  }
}

void DeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, const uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0;
}

void DeviceDriver::EnterStandby() {}

void* DeviceDriver::GetComponent(const char* pchComponentNameAndVersion) {
  return nullptr;
}

vr::DriverPose_t DeviceDriver::GetPose() {
  if (hasActivated_) return controllerPose_->UpdatePose();

  return vr::DriverPose_t{0};
}

std::string DeviceDriver::GetSerialNumber() {
  return serialNumber_;
}

int32_t DeviceDriver::GetDeviceId() const {
  return deviceId_;
}

bool DeviceDriver::IsActive() {
  return hasActivated_;
}

void DeviceDriver::PoseUpdateThread() const {
  while (hasActivated_) {
    vr::DriverPose_t pose = controllerPose_->UpdatePose();
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(deviceId_, pose, sizeof(vr::DriverPose_t));

    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }

  DriverLog("Closing pose thread...");
}

bool DeviceDriver::IsRightHand() const {
  return configuration_.role == vr::TrackedControllerRole_RightHand;
}

void DeviceDriver::StartDevice() {
  StartingDevice();

  vr::VRDriverInput()->UpdateSkeletonComponent(skeletalComponentHandle_, vr::VRSkeletalMotionRange_WithoutController, handTransforms_, NUM_BONES);
  vr::VRDriverInput()->UpdateSkeletonComponent(skeletalComponentHandle_, vr::VRSkeletalMotionRange_WithController, handTransforms_, NUM_BONES);

  communicationManager_->BeginListener([&](VRInputData data) {
    try {
      boneAnimator_->ComputeSkeletonTransforms(handTransforms_, data, IsRightHand());
      vr::VRDriverInput()->UpdateSkeletonComponent(skeletalComponentHandle_, vr::VRSkeletalMotionRange_WithoutController, handTransforms_, NUM_BONES);
      vr::VRDriverInput()->UpdateSkeletonComponent(skeletalComponentHandle_, vr::VRSkeletalMotionRange_WithController, handTransforms_, NUM_BONES);

      HandleInput(data);

      if (configuration_.poseConfiguration.calibrationButtonEnabled) {
        if (data.calibrate) {
          if (!controllerPose_->IsCalibrating()) controllerPose_->StartCalibration(CalibrationMethod::Hardware);
        } else {
          if (controllerPose_->IsCalibrating()) controllerPose_->CompleteCalibration(CalibrationMethod::Hardware);
        }
      }

    } catch (const std::exception&) {
      DebugDriverLog("Exception caught while parsing comm data");
    }
  });

  poseUpdateThread_ = std::thread(&DeviceDriver::PoseUpdateThread, this);
}

void DeviceDriver::OnEvent(vr::VREvent_t vrEvent) const {
  if (vrEvent.eventType == vr::EVREventType::VREvent_Input_HapticVibration && haptic_ == vrEvent.data.hapticVibration.componentHandle)
    communicationManager_->QueueSend(VRHapticData(vrEvent.data.hapticVibration));
}
