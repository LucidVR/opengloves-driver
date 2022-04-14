#include "DeviceDriver/DeviceDriver.h"

#include <utility>

#include "Communication/BTSerialCommunicationManager.h"
#include "Communication/NamedPipeCommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"
#include "DriverLog.h"
#include "Encode/AlphaEncodingManager.h"
#include "Encode/LegacyEncodingManager.h"

DeviceDriver::DeviceDriver(VRDeviceConfiguration configuration)
    : configuration_(std::move(configuration)),
      skeletalComponentHandle_(),
      handTransforms_(),
      isRunning_(false),
      deviceId_(vr::k_unTrackedDeviceIndexInvalid) {}

vr::EVRInitError DeviceDriver::Activate(uint32_t unObjectId) {
  deviceId_ = unObjectId;

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

  StartDevice();

  isRunning_ = true;

  return vr::VRInitError_None;
}

void DeviceDriver::Deactivate() {
  DriverLog("Deactivating device");
  if (isRunning_.exchange(false)) {
    StopDeviceComponents();

    StoppingDevice();
  }

  DriverLog("Finished deactivating device, invalidating pose...");
}

void DeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, const uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0;
}

void DeviceDriver::EnterStandby() {}

void* DeviceDriver::GetComponent(const char* pchComponentNameAndVersion) {
  return nullptr;
}

vr::DriverPose_t DeviceDriver::GetPose() {
  if (isActive_) return controllerPose_->UpdatePose();

  return vr::DriverPose_t{0};
}

std::string DeviceDriver::GetSerialNumber() {
  return configuration_.serialNumber;
}

int32_t DeviceDriver::GetDeviceId() const {
  return deviceId_;
}

bool DeviceDriver::IsActive() {
  return isRunning_;
}

void DeviceDriver::PoseUpdateThread() const {
  while (isActive_) {
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
  isActive_ = true;

  StartingDevice();

  SetupDeviceComponents();

  vr::VRDriverInput()->UpdateSkeletonComponent(skeletalComponentHandle_, vr::VRSkeletalMotionRange_WithoutController, handTransforms_, NUM_BONES);
  vr::VRDriverInput()->UpdateSkeletonComponent(skeletalComponentHandle_, vr::VRSkeletalMotionRange_WithController, handTransforms_, NUM_BONES);
}

void DeviceDriver::OnEvent(vr::VREvent_t vrEvent) const {
  if (vrEvent.eventType == vr::EVREventType::VREvent_Input_HapticVibration && haptic_ == vrEvent.data.hapticVibration.componentHandle)
    communicationManager_->QueueSend(VRHapticData(vrEvent.data.hapticVibration));
}

void DeviceDriver::UpdateDeviceConfiguration(VRDeviceConfiguration configuration) {
  configuration_ = std::move(configuration);

  DriverLog("Attempting to stop device components...");

  StopDeviceComponents();

  DriverLog("Restarting device components...");

  isActive_ = true;
  SetupDeviceComponents();

  DriverLog("Restarted device components successfully");
}

void DeviceDriver::SetupDeviceComponents() {
  const VRCommunicationConfiguration& communicationConfiguration = configuration_.communicationConfiguration;
  std::unique_ptr<EncodingManager> encodingManager;

  switch (communicationConfiguration.encodingConfiguration.encodingProtocol) {
    case VREncodingProtocol::Legacy: {
      DriverLog("Using legacy encoding");
      encodingManager = std::make_unique<LegacyEncodingManager>(communicationConfiguration.encodingConfiguration);

      break;
    }

    case VREncodingProtocol::Alpha: {
      DriverLog("Using alpha encoding");
      encodingManager = std::make_unique<AlphaEncodingManager>(communicationConfiguration.encodingConfiguration);

      break;
    }
  }

  switch (communicationConfiguration.communicationProtocol) {
    case VRCommunicationProtocol::NamedPipe: {
      DriverLog("Using named pipe communication");
      communicationManager_ = std::make_unique<NamedPipeCommunicationManager>(communicationConfiguration);

      break;
    }

    case VRCommunicationProtocol::BtSerial: {
      DriverLog("Using bluetooth serial communication");
      communicationManager_ = std::make_unique<BTSerialCommunicationManager>(communicationConfiguration, std::move(encodingManager));

      break;
    }

    case VRCommunicationProtocol::Serial: {
      DriverLog("Using usb serial communication");
      communicationManager_ = std::make_unique<SerialCommunicationManager>(communicationConfiguration, std::move(encodingManager));

      break;
    }
  }

  controllerPose_ = std::make_unique<ControllerPose>(configuration_.role, std::string(c_deviceManufacturer), configuration_.poseConfiguration);

  boneAnimator_ = std::make_unique<BoneAnimator>(GetDriverPath() + R"(\resources\anims\glove_anim.glb)");
  boneAnimator_->LoadDefaultSkeletonByHand(handTransforms_, configuration_.role == vr::ETrackedControllerRole::TrackedControllerRole_RightHand);

  ffbProvider_ = std::make_unique<FFBListener>(
      [&](const VRFFBData data) {
        // Queue the force feedback data for sending.
        communicationManager_->QueueSend(data);
      },
      configuration_.role);

  ffbProvider_->Start();

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

void DeviceDriver::StopDeviceComponents() {
  if (isActive_.exchange(false)) {
    ffbProvider_->Stop();

    communicationManager_->Disconnect();

    poseUpdateThread_.join();
  }

  vr::DriverPose_t pose;
  pose.deviceIsConnected = false;
  pose.poseIsValid = false;
  vr::VRServerDriverHost()->TrackedDevicePoseUpdated(deviceId_, pose, sizeof(vr::DriverPose_t));
}

void DeviceDriver::DisableDevice() {
  DriverLog("Disabling device components...");
  StopDeviceComponents();
}
