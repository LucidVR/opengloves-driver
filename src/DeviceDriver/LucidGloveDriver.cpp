#include "DeviceDriver/LucidGloveDriver.h"

static const char* c_deviceControllerType = "lucidgloves";
static const char* c_deviceModelNumber = "lucidgloves1";
static const char* c_basePosePath = "/pose/raw";
static const char* c_inputProfilePath = "{openglove}/input/openglove_profile.json";

LucidGloveDeviceDriver::LucidGloveDeviceDriver(
    std::unique_ptr<CommunicationManager> communicationManager,
    std::shared_ptr<BoneAnimator> boneAnimator,
    const std::string& serialNumber,
    const VRDeviceConfiguration configuration)
    : DeviceDriver(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration), inputComponentHandles_() {}

void LucidGloveDeviceDriver::HandleInput(const VRInputData data) {
  // clang-format off
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::JoyX)], data.joyX, 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::JoyY)], data.joyY, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::JoyBtn)], data.joyButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnTrg)], data.trgButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnA)], data.aButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnB)], data.bButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::GesGrab)], data.grab, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::GesPinch)], data.pinch, 0);

  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgThumb)], boneAnimator_->GetAverageCurlValue(data.flexion[0]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgIndex)], boneAnimator_->GetAverageCurlValue(data.flexion[1]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgMiddle)], boneAnimator_->GetAverageCurlValue(data.flexion[2]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgRing)], boneAnimator_->GetAverageCurlValue(data.flexion[3]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgPinky)], boneAnimator_->GetAverageCurlValue(data.flexion[4]), 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnMenu)], data.menu, 0);
  // clang-format on
}

void LucidGloveDeviceDriver::SetupProps(vr::PropertyContainerHandle_t& props) {
  const bool isRightHand = IsRightHand();

  // clang-format off
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, 2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, c_inputProfilePath);    // tell OpenVR where to get your driver's Input Profile
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, configuration_.role);  // tells OpenVR what kind of device this is
  vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, GetSerialNumber().c_str());
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, c_deviceModelNumber);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceDriverManufacturer);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, c_deviceControllerType);

  vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::JoyX)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::JoyY)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/joystick/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::JoyBtn)]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnTrg)]);

  vr::VRDriverInput()->CreateBooleanComponent(props, isRightHand ? "/input/A/click" : "/input/system/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnA)]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/B/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnB)]);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grab/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::GesGrab)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/pinch/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::GesPinch)]);

  vr::VRDriverInput()->CreateHapticComponent(props, "output/haptic", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::Haptic)]);

  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/thumb", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgThumb)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgIndex)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgMiddle)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgRing)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::TrgPinky)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &inputComponentHandles_[static_cast<int>(LucidGloveDeviceComponentIndex::BtnMenu)]);
  // clang-format on
}

void LucidGloveDeviceDriver::StartingDevice() {}

void LucidGloveDeviceDriver::StoppingDevice() {}
