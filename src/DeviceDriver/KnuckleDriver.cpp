#include "DeviceDriver/KnuckleDriver.h"

#include <utility>

KnuckleDeviceDriver::KnuckleDeviceDriver(
    std::unique_ptr<CommunicationManager> communicationManager,
    std::shared_ptr<BoneAnimator> boneAnimator,
    std::string serialNumber,
    bool approximateThumb,
    const VRDeviceConfiguration configuration)
    : DeviceDriver(std::move(communicationManager), std::move(boneAnimator), std::move(serialNumber), configuration),
      inputComponentHandles_(),
      haptic_(),
      approximateThumb_(approximateThumb) {}

void KnuckleDeviceDriver::HandleInput(const VRInputData data) {
  // clang-format off
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickX)], data.joyX, 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickY)], data.joyY, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickClick)], data.joyButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickTouch)], data.joyButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TriggerClick)], data.trgButton, 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TriggerValue)], boneAnimator_->GetAverageCurlValue(data.flexion[1]), 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::AClick)], data.aButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ATouch)], data.aButton || (approximateThumb_ && boneAnimator_->GetAverageCurlValue(data.flexion[0]) > 0.6), 0); //Thumb approximation

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::BClick)], data.bButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::BTouch)], data.bButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::GripTouch)], data.grab, 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::GripForce)], data.grab, 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::GripValue)], data.grab, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::SystemClick)], data.menu, 0);

  // We don't have a thumb on the index
  // vr::VRDriverInput()->UpdateScalarComponent(_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMB], data.flexion[0], 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerIndex)], boneAnimator_->GetAverageCurlValue(data.flexion[1]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerMiddle)], boneAnimator_->GetAverageCurlValue(data.flexion[2]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerRing)], boneAnimator_->GetAverageCurlValue(data.flexion[3]), 0);
  vr::VRDriverInput()->UpdateScalarComponent(inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerPinky)], boneAnimator_->GetAverageCurlValue(data.flexion[4]), 0);
  // clang-format on
}

void KnuckleDeviceDriver::SetupProps(vr::PropertyContainerHandle_t& props) {
  // clang-format off
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, 2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, GetSerialNumber().c_str());
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_WillDriftInYaw_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_DeviceIsWireless_Bool, true);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_DeviceIsCharging_Bool, false);
  vr::VRProperties()->SetFloatProperty(props, vr::Prop_DeviceBatteryPercentage_Float, 1.f);  // Always charged

  vr::HmdMatrix34_t l_matrix = {-1.f, 0.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, -1.f, 0.f, 0.f};
  vr::VRProperties()->SetProperty(props, vr::Prop_StatusDisplayTransform_Matrix34, &l_matrix, sizeof(vr::HmdMatrix34_t), vr::k_unHmdMatrix34PropertyTag);

  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Firmware_UpdateAvailable_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Firmware_ManualUpdate_Bool, false);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_Firmware_ManualUpdateURL_String, "https://developer.valvesoftware.com/wiki/SteamVR/HowTo_Update_Firmware");
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_DeviceProvidesBatteryStatus_Bool, true);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_DeviceCanPowerOff_Bool, false);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_Controller);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Identifiable_Bool, true);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Firmware_RemindUpdate_Bool, false);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasDisplayComponent_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasCameraComponent_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasDriverDirectModeComponent_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasVirtualDisplayComponent_Bool, false);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, 2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, IsRightHand() ? "Knuckles Right" : "Knuckles Left");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String, IsRightHand() ? "{indexcontroller}valve_controller_knu_1_0_right" : "{indexcontroller}valve_controller_knu_1_0_left");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceDriverManufacturer);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_TrackingFirmwareVersion_String, "1562916277 watchman@ValveBuilder02 2019-07-12 FPGA 538(2.26/10/2) BL 0 VRC 1562916277 Radio 1562882729");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_HardwareRevision_String, "product 17 rev 14.1.9 lot 2019/4/20 0");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ConnectedWirelessDongle_String, "C2F75F5986-DIY");
  vr::VRProperties()->SetUint64Property(props, vr::Prop_HardwareRevision_Uint64, 286130441U);
  vr::VRProperties()->SetUint64Property(props, vr::Prop_FirmwareVersion_Uint64, 1562916277U);
  vr::VRProperties()->SetUint64Property(props, vr::Prop_FPGAVersion_Uint64, 538U);
  vr::VRProperties()->SetUint64Property(props, vr::Prop_VRCVersion_Uint64, 1562916277U);
  vr::VRProperties()->SetUint64Property(props, vr::Prop_RadioVersion_Uint64, 1562882729U);
  vr::VRProperties()->SetUint64Property(props, vr::Prop_DongleVersion_Uint64, 1558748372U);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_Firmware_ProgrammingTarget_String, IsRightHand() ? "LHR-E217CD01" : "LHR-E217CD00");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ResourceRoot_String, "indexcontroller");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_RegisteredDeviceType_String, IsRightHand() ? "valve/index_controllerLHR-E217CD01" : "valve/index_controllerLHR-E217CD00");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, "{indexcontroller}/input/index_controller_profile.json");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceOff_String, IsRightHand() ? "{openglove}/icons/right_controller_status_off.png" : "{openglove}/icons/left_controller_status_off.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceSearching_String, IsRightHand() ? "{openglove}/icons/right_controller_status_searching.gif" : "{openglove}/icons/left_controller_status_searching.gif");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceSearchingAlert_String, IsRightHand() ? "{openglove}/icons/right_controller_status_searching_alert.gif" : "{openglove}/icons/left_controller_status_searching_alert.gif");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReady_String, IsRightHand() ? "{openglove}/icons/right_controller_status_ready.png" : "{openglove}/icons/left_controller_status_ready.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReadyAlert_String, IsRightHand() ? "{openglove}/icons/right_controller_status_ready_alert.png" : "{openglove}/icons/left_controller_status_ready_alert.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceNotReady_String, IsRightHand() ? "{openglove}/icons/right_controller_status_error.png" : "{openglove}/icons/left_controller_status_error.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceStandby_String, IsRightHand() ? "{openglove}/icons/right_controller_status_off.png" : "{openglove}/icons/left_controller_status_off.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceAlertLow_String, IsRightHand() ? "{openglove}/icons/right_controller_status_ready_low.png" : "{openglove}/icons/left_controller_status_ready_low.png");

  vr::VRProperties()->SetInt32Property(props, vr::Prop_Axis2Type_Int32, vr::k_eControllerAxis_Trigger);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, "knuckles");

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::SystemClick)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/touch", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::SystemTouch)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TriggerClick)]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trigger/value", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TriggerValue)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TrackpadX)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TrackpadY)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trackpad/touch", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TrackpadTouch)]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/force", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::TrackpadForce)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grip/touch", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::GripTouch)]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/force", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::GripForce)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/value", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::GripValue)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/click", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickClick)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/touch", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickTouch)]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/x", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickX)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/y", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ThumbstickY)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/click", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::AClick)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/touch", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::ATouch)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/click", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::BClick)]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/touch", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::BTouch)]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerIndex)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerMiddle)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerRing)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &inputComponentHandles_[static_cast<int>(KnuckleDeviceComponentIndex::FingerPinky)], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateHapticComponent(props, "/output/haptic", &haptic_);
  // clang-format on
}

void KnuckleDeviceDriver::StartingDevice() {
  if (!configuration_.feedbackEnabled) return;

  ffbProvider_ = std::make_unique<FFBListener>(
      [&](const VRFFBData data) {
        // Queue the force feedback data for sending.
        communicationManager_->QueueSend(data);
      },
      configuration_.role);

  ffbProvider_->Start();
}

void KnuckleDeviceDriver::StoppingDevice() {
  if (ffbProvider_) {
    ffbProvider_->Stop();
  }
}
