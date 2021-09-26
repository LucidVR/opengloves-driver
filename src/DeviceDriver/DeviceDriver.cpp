#include "DeviceDriver/DeviceDriver.h"

#include "DriverLog.h"

DeviceDriver::DeviceDriver(std::unique_ptr<CommunicationManager> communicationManager, std::shared_ptr<BoneAnimator> boneAnimator, std::string serialNumber,
                           VRDeviceConfiguration configuration)
    : m_communicationManager(std::move(communicationManager)),
      m_boneAnimator(std::move(boneAnimator)),
      m_serialNumber(serialNumber),
      m_configuration(configuration),
      m_skeletalComponentHandle(),
      m_handTransforms(),
      m_hasActivated(false),
      m_driverId(vr::k_unTrackedDeviceIndexInvalid) {
  // copy a default bone transform to our hand transform for use in finger positioning later
  std::copy(std::begin(IsRightHand() ? rightOpenPose : leftOpenPose), std::end(IsRightHand() ? rightOpenPose : leftOpenPose), std::begin(m_handTransforms));
}

vr::EVRInitError DeviceDriver::Activate(uint32_t unObjectId) {
  m_driverId = unObjectId;
  m_controllerPose = std::make_unique<ControllerPose>(m_configuration.role, std::string(c_deviceDriverManufacturer), m_configuration.poseConfiguration);

  vr::PropertyContainerHandle_t props =
      vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId);  // this gets a container object where you store all the information about your driver

  SetupProps(props);

  vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(props, IsRightHand() ? "/input/skeleton/right" : "/input/skeleton/left",
                                                                         IsRightHand() ? "/skeleton/hand/right" : "/skeleton/hand/left", "/pose/raw",
                                                                         vr::VRSkeletalTracking_Partial, m_handTransforms, NUM_BONES, &m_skeletalComponentHandle);

  if (error != vr::VRInputError_None) {
    DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
  }

  StartDevice();

  m_hasActivated = true;

  return vr::VRInitError_None;
}

void DeviceDriver::Deactivate() {
  if (m_hasActivated) {
    StoppingDevice();
    m_communicationManager->Disconnect();
    m_driverId = vr::k_unTrackedDeviceIndexInvalid;
    m_hasActivated = false;
  }
}

void DeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0;
}

void DeviceDriver::EnterStandby() {}

void* DeviceDriver::GetComponent(const char* pchComponentNameAndVersion) { return nullptr; }

vr::DriverPose_t DeviceDriver::GetPose() {
  if (m_hasActivated) return m_controllerPose->UpdatePose();

  vr::DriverPose_t pose = {0};
  return pose;
}

std::string DeviceDriver::GetSerialNumber() { return m_serialNumber; }

bool DeviceDriver::IsActive() { return m_hasActivated; }

void DeviceDriver::RunFrame() {
  if (m_hasActivated) {
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_driverId, m_controllerPose->UpdatePose(), sizeof(vr::DriverPose_t));
  }
}

bool DeviceDriver::IsRightHand() const { return m_configuration.role == vr::TrackedControllerRole_RightHand; }

void DeviceDriver::StartDevice() {
  StartingDevice();

  vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, IsRightHand() ? rightOpenPose : leftOpenPose,
                                               NUM_BONES);
  vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, IsRightHand() ? rightOpenPose : leftOpenPose,
                                               NUM_BONES);

  m_communicationManager->BeginListener([&](VRInputData datas) {
    try {
      m_boneAnimator->ComputeSkeletonTransforms(m_handTransforms, datas.flexion, IsRightHand());
      vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
      vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, m_handTransforms, NUM_BONES);

      HandleInput(datas);

      if (m_configuration.poseConfiguration.calibrationButtonEnabled) {
        if (datas.calibrate) {
          if (!m_controllerPose->isCalibrating()) m_controllerPose->StartCalibration(CalibrationMethod::HARDWARE);
        } else {
          if (m_controllerPose->isCalibrating()) m_controllerPose->CompleteCalibration(CalibrationMethod::HARDWARE);
        }
      }

    } catch (const std::exception&) {
      DebugDriverLog("Exception caught while parsing comm data");
    }
  });
}
