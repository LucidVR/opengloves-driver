#include "DeviceDriver/LucidGloveDriver.h"

#include "DriverLog.h"

static const char* c_deviceControllerType = "lucidgloves";
static const char* c_deviceModelNumber = "lucidgloves1";
static const char* c_basePosePath = "/pose/raw";
static const char* c_inputProfilePath = "{openglove}/input/openglove_profile.json";

LucidGloveDeviceDriver::LucidGloveDeviceDriver(std::unique_ptr<CommunicationManager> communicationManager, std::shared_ptr<BoneAnimator> boneAnimator,
                                               std::string serialNumber, VRDeviceConfiguration configuration)
    : DeviceDriver(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration), m_inputComponentHandles() {}

void LucidGloveDeviceDriver::HandleInput(VRInputData datas) {
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_JOY_X], datas.joyX, 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_JOY_Y], datas.joyY, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_JOY_BTN], datas.joyButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_TRG], datas.trgButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_A], datas.aButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_B], datas.bButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_GES_GRAB], datas.grab, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_GES_PINCH], datas.pinch, 0);

  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_THUMB], datas.flexion[0], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_INDEX], datas.flexion[1], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_MIDDLE], datas.flexion[2], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_RING], datas.flexion[3], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_PINKY], datas.flexion[4], 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_MENU], datas.menu, 0);
}

void LucidGloveDeviceDriver::SetupProps(vr::PropertyContainerHandle_t& props) {
  const bool isRightHand = IsRightHand();

  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, c_inputProfilePath);    // tell OpenVR where to get your driver's Input Profile
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, m_configuration.role);  // tells OpenVR what kind of device this is
  vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, GetSerialNumber().c_str());
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, c_deviceModelNumber);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceDriverManufacturer);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, c_deviceControllerType);

  vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_JOY_X],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_JOY_Y],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/joystick/click", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_JOY_BTN]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_TRG]);

  vr::VRDriverInput()->CreateBooleanComponent(props, isRightHand ? "/input/A/click" : "/input/system/click",
                                              &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_A]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/B/click", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_B]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grab/click", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_GES_GRAB]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/pinch/click", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_GES_PINCH]);

  vr::VRDriverInput()->CreateHapticComponent(props, "output/haptic", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_HAPTIC]);

  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/thumb", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_THUMB],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_INDEX],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_MIDDLE],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_RING],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_TRG_PINKY],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &m_inputComponentHandles[(int)LucidGloveDeviceComponentIndex::COMP_BTN_MENU]);
}

void LucidGloveDeviceDriver::StartingDevice() {}

void LucidGloveDeviceDriver::StoppingDevice() {}
