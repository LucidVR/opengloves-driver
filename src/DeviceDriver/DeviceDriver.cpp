#include "DeviceDriver/DeviceDriver.h"

#include <utility>

#include "DriverLog.h"

DeviceDriver::DeviceDriver(
    std::unique_ptr<CommunicationManager> communicationManager,
    std::shared_ptr<BoneAnimator> boneAnimator,
    std::string serialNumber,
    const VRDeviceConfiguration configuration)
    : _communicationManager(std::move(communicationManager)),
      _boneAnimator(std::move(boneAnimator)),
      _configuration(configuration),
      _serialNumber(std::move(serialNumber)),
      _skeletalComponentHandle(),
      _handTransforms(),
      _hasActivated(false),
      _driverId(vr::k_unTrackedDeviceIndexInvalid) {
  // copy a default bone transform to our hand transform for use in finger positioning later
  std::copy(
      std::begin(IsRightHand() ? rightOpenPose : leftOpenPose), std::end(IsRightHand() ? rightOpenPose : leftOpenPose), std::begin(_handTransforms));
}

vr::EVRInitError DeviceDriver::Activate(uint32_t unObjectId) {
  _driverId = unObjectId;
  _controllerPose = std::make_unique<ControllerPose>(_configuration.role, std::string(c_deviceDriverManufacturer), _configuration.poseConfiguration);

  vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(
      _driverId);  // this gets a container object where you store all the information about your driver

  SetupProps(props);

  if (const vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(
          props,
          IsRightHand() ? "/input/skeleton/right" : "/input/skeleton/left",
          IsRightHand() ? "/skeleton/hand/right" : "/skeleton/hand/left",
          "/pose/raw",
          vr::VRSkeletalTracking_Partial,
          _handTransforms,
          NUM_BONES,
          &_skeletalComponentHandle);
      error != vr::VRInputError_None) {
    DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
  }

  StartDevice();

  _hasActivated = true;

  return vr::VRInitError_None;
}

void DeviceDriver::Deactivate() {
  if (_hasActivated) {
    StoppingDevice();
    _communicationManager->Disconnect();
    _driverId = vr::k_unTrackedDeviceIndexInvalid;
    _hasActivated = false;
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
  if (_hasActivated) return _controllerPose->UpdatePose();

  return vr::DriverPose_t{0};
}

std::string DeviceDriver::GetSerialNumber() {
  return _serialNumber;
}

bool DeviceDriver::IsActive() {
  return _hasActivated;
}

void DeviceDriver::RunFrame() {
  if (_hasActivated) {
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(_driverId, _controllerPose->UpdatePose(), sizeof(vr::DriverPose_t));
  }
}

bool DeviceDriver::IsRightHand() const {
  return _configuration.role == vr::TrackedControllerRole_RightHand;
}

void DeviceDriver::StartDevice() {
  StartingDevice();

  vr::VRDriverInput()->UpdateSkeletonComponent(
      _skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, IsRightHand() ? rightOpenPose : leftOpenPose, NUM_BONES);
  vr::VRDriverInput()->UpdateSkeletonComponent(
      _skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, IsRightHand() ? rightOpenPose : leftOpenPose, NUM_BONES);

  _communicationManager->BeginListener([&](VRInputData data) {
    try {
      _boneAnimator->ComputeSkeletonTransforms(_handTransforms, data.flexion, IsRightHand());
      vr::VRDriverInput()->UpdateSkeletonComponent(_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, _handTransforms, NUM_BONES);
      vr::VRDriverInput()->UpdateSkeletonComponent(_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, _handTransforms, NUM_BONES);

      HandleInput(data);

      if (_configuration.poseConfiguration.calibrationButtonEnabled) {
        if (data.calibrate) {
          if (!_controllerPose->IsCalibrating()) _controllerPose->StartCalibration(CalibrationMethod::Hardware);
        } else {
          if (_controllerPose->IsCalibrating()) _controllerPose->CompleteCalibration(CalibrationMethod::Hardware);
        }
      }

    } catch (const std::exception&) {
      DebugDriverLog("Exception caught while parsing comm data");
    }
  });
}
