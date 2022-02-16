#include "DeviceDriver/DeviceDriver.h"

#include <utility>

#include "DriverLog.h"

DeviceDriver::DeviceDriver(
    std::unique_ptr<CommunicationManager> communicationManager,
    std::unique_ptr<BoneAnimator> boneAnimator,
    std::string serialNumber,
    const VRDeviceConfiguration configuration)
    : communicationManager_(std::move(communicationManager)),
      boneAnimator_(std::move(boneAnimator)),
      configuration_(configuration),
      serialNumber_(std::move(serialNumber)),
      skeletalComponentHandle_(),
      handTransforms_(),
      hasActivated_(false),
      driverId_(vr::k_unTrackedDeviceIndexInvalid) {
  // Load in a default skeleton
  boneAnimator_->LoadDefaultSkeletonByHand(handTransforms_, configuration_.role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand);
}

vr::EVRInitError DeviceDriver::Activate(uint32_t unObjectId) {
  driverId_ = unObjectId;

  vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(driverId_);

  SetupProps(props);

  const vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(
      props,
      IsRightHand() ? "/input/skeleton/right" : "/input/skeleton/left",
      IsRightHand() ? "/skeleton/hand/right" : "/skeleton/hand/left",
      "/pose/raw",
      vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Full,
      handTransforms_,
      NUM_BONES,
      &skeletalComponentHandle_);

  if (error != vr::VRInputError_None) {
    DriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
  }

  StartDevice();

  hasActivated_ = true;

  return vr::VRInitError_None;
}

void DeviceDriver::Deactivate() {
  if (!hasActivated_.exchange(false)) return;

  StoppingDevice();

  communicationManager_->Disconnect();
  if (ffbProvider_) ffbProvider_->Stop();

  hasActivated_ = false;
  driverId_ = vr::k_unTrackedDeviceIndexInvalid;
}

void DeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, const uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0;
}

void DeviceDriver::EnterStandby() {}

void* DeviceDriver::GetComponent(const char* pchComponentNameAndVersion) {
  return nullptr;
}

std::string DeviceDriver::GetSerialNumber() {
  return serialNumber_;
}

bool DeviceDriver::IsActive() {
  return hasActivated_;
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

    } catch (const std::exception&) {
      DebugDriverLog("Exception caught while parsing comm data");
    }
  });

  if (configuration_.feedbackEnabled) {
    ffbProvider_ = std::make_unique<FFBListener>(
        [&](const VRFFBData data) {
          // Queue the force feedback data for sending.
          communicationManager_->QueueSend(data);
        },
        configuration_.role);

    ffbProvider_->Start();
  }
}
