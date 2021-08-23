#include "DeviceDriver/LucidGloveDriver.h"

#include "DriverLog.h"

static const char* c_deviceManufacturer = "LucidVR";
static const char* c_deviceControllerType = "lucidgloves";
static const char* c_deviceModelNumber = "lucidgloves1";
static const char* c_basePosePath = "/pose/raw";
static const char* c_inputProfilePath = "{openglove}/input/openglove_profile.json";

enum ComponentIndex : int {
  COMP_JOY_X = 0,
  COMP_JOY_Y = 1,
  COMP_JOY_BTN = 2,
  COMP_BTN_TRG = 3,
  COMP_BTN_A = 4,
  COMP_BTN_B = 5,
  COMP_GES_GRAB = 6,
  COMP_GES_PINCH = 7,
  COMP_HAPTIC = 8,
  COMP_TRG_THUMB = 9,
  COMP_TRG_INDEX = 10,
  COMP_TRG_MIDDLE = 11,
  COMP_TRG_RING = 12,
  COMP_TRG_PINKY = 13,
  COMP_BTN_MENU = 14,
};

LucidGloveDeviceDriver::LucidGloveDeviceDriver(VRDeviceConfiguration_t configuration, std::unique_ptr<CommunicationManager> communicationManager,
                                               std::string serialNumber, std::shared_ptr<BoneAnimator> boneAnimator)
    : m_configuration(configuration),
      m_communicationManager(std::move(communicationManager)),
      m_boneAnimator(std::move(boneAnimator)),
      m_serialNumber(serialNumber),
      m_driverId(-1),
      m_hasActivated(false) {
  // copy a default bone transform to our hand transform for use in finger positioning later
  std::copy(std::begin(m_configuration.role == vr::TrackedControllerRole_RightHand ? rightOpenPose : leftOpenPose),
            std::end(m_configuration.role == vr::TrackedControllerRole_RightHand ? rightOpenPose : leftOpenPose), std::begin(m_handTransforms));
}

bool LucidGloveDeviceDriver::IsRightHand() const { return m_configuration.role == vr::TrackedControllerRole_RightHand; }

std::string LucidGloveDeviceDriver::GetSerialNumber() { return m_serialNumber; }

bool LucidGloveDeviceDriver::IsActive() { return m_hasActivated; }

vr::EVRInitError LucidGloveDeviceDriver::Activate(uint32_t unObjectId) {
  const bool isRightHand = IsRightHand();

  m_driverId = unObjectId;  // unique ID for your driver
  m_controllerPose = std::make_unique<ControllerPose>(m_configuration.role, std::string(c_deviceManufacturer), m_configuration.poseConfiguration);

  vr::PropertyContainerHandle_t props =
      vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId);  // this gets a container object where you store all the information about your driver

  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, c_inputProfilePath);    // tell OpenVR where to get your driver's Input Profile
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, m_configuration.role);  // tells OpenVR what kind of device this is
  vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, GetSerialNumber().c_str());
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, c_deviceModelNumber);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceManufacturer);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, c_deviceControllerType);

  vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &m_inputComponentHandles[ComponentIndex::COMP_JOY_X], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &m_inputComponentHandles[ComponentIndex::COMP_JOY_Y], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/joystick/click", &m_inputComponentHandles[ComponentIndex::COMP_JOY_BTN]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_TRG]);

  vr::VRDriverInput()->CreateBooleanComponent(props, isRightHand ? "/input/A/click" : "/input/system/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_A]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/B/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_B]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grab/click", &m_inputComponentHandles[ComponentIndex::COMP_GES_GRAB]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/pinch/click", &m_inputComponentHandles[ComponentIndex::COMP_GES_PINCH]);

  vr::VRDriverInput()->CreateHapticComponent(props, "output/haptic", &m_inputComponentHandles[ComponentIndex::COMP_HAPTIC]);

  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/thumb", &m_inputComponentHandles[ComponentIndex::COMP_TRG_THUMB], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &m_inputComponentHandles[ComponentIndex::COMP_TRG_INDEX], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &m_inputComponentHandles[ComponentIndex::COMP_TRG_MIDDLE], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &m_inputComponentHandles[ComponentIndex::COMP_TRG_RING], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &m_inputComponentHandles[ComponentIndex::COMP_TRG_PINKY], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_MENU]);

  // Create the skeletal component and save the handle for later use//

  vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(
      props, isRightHand ? "/input/skeleton/right" : "/input/skeleton/left", isRightHand ? "/skeleton/hand/right" : "/skeleton/hand/left", "/pose/raw",
      vr::VRSkeletalTracking_Partial, isRightHand ? rightOpenPose : leftOpenPose, NUM_BONES, &m_skeletalComponentHandle);

  if (error != vr::VRInputError_None) {
    // Handle failure case
    DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
  }

  StartDevice();

  m_hasActivated = true;

  return vr::VRInitError_None;
}

void LucidGloveDeviceDriver::StartDevice() {
  vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
  vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, m_handTransforms, NUM_BONES);

  m_communicationManager->BeginListener([&](VRCommData_t datas) {
    try {
      m_boneAnimator->ComputeSkeletonTransforms(m_handTransforms, datas.flexion, IsRightHand());

      vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
      vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, m_handTransforms, NUM_BONES);

      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_X], datas.joyX, 0);
      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_Y], datas.joyY, 0);

      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_BTN], datas.joyButton, 0);
      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_TRG], datas.trgButton, 0);
      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_A], datas.aButton, 0);
      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_B], datas.bButton, 0);

      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_GES_GRAB], datas.grab, 0);
      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_GES_PINCH], datas.pinch, 0);

      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_THUMB], datas.flexion[0], 0);
      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_INDEX], datas.flexion[1], 0);
      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_MIDDLE], datas.flexion[2], 0);
      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_RING], datas.flexion[3], 0);
      vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_PINKY], datas.flexion[4], 0);

      vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_MENU], datas.menu, 0);

      if (datas.calibrate) {
        if (!m_controllerPose->isCalibrating()) m_controllerPose->StartCalibration();
      } else {
        if (m_controllerPose->isCalibrating()) m_controllerPose->CompleteCalibration();
      }

    } catch (const std::exception&) {
      DriverLog("Exception caught while parsing comm data");
    }
  });
}

vr::DriverPose_t LucidGloveDeviceDriver::GetPose() {
  if (m_hasActivated) return m_controllerPose->UpdatePose();

  vr::DriverPose_t pose = {0};
  return pose;
}

void LucidGloveDeviceDriver::RunFrame() {
  if (m_hasActivated) {
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_driverId, m_controllerPose->UpdatePose(), sizeof(vr::DriverPose_t));
  }
}

void LucidGloveDeviceDriver::Deactivate() {
  if (m_hasActivated) {
    m_communicationManager->Disconnect();
    m_driverId = vr::k_unTrackedDeviceIndexInvalid;
  }
}

void* LucidGloveDeviceDriver::GetComponent(const char* pchComponentNameAndVersion) { return nullptr; }

void LucidGloveDeviceDriver::EnterStandby() {}

void LucidGloveDeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) {
    pchResponseBuffer[0] = 0;
  }
}