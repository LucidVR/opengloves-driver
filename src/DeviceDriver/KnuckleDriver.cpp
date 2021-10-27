#include "DeviceDriver/KnuckleDriver.h"

#include <utility>

#include "DriverLog.h"

KnuckleDeviceDriver::KnuckleDeviceDriver(std::unique_ptr<CommunicationManager> communicationManager, std::shared_ptr<BoneAnimator> boneAnimator, std::string serialNumber,
                                         VRDeviceConfiguration configuration)
    : DeviceDriver(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration), m_inputComponentHandles(), m_haptic(), m_ffbProvider() {}

void KnuckleDeviceDriver::HandleInput(VRInputData datas) {
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_X], datas.joyX, 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_Y], datas.joyY, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_CLICK], datas.joyButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_TOUCH], datas.joyButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRIGGER_CLICK], datas.trgButton, 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRIGGER_VALUE], datas.flexion[1], 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::A_CLICK], datas.aButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::A_TOUCH], datas.aButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::B_CLICK], datas.bButton, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::B_TOUCH], datas.bButton, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::GRIP_FORCE], datas.grab, 0);
  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::GRIP_TOUCH], datas.grab, 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::GRIP_VALUE], datas.grab, 0);

  vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::SYSTEM_CLICK], datas.menu, 0);

  // We don't have a thumb on the index
  // vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMB], datas.flexion[0], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_INDEX], datas.flexion[1], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_MIDDLE], datas.flexion[2], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_RING], datas.flexion[3], 0);
  vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_PINKY], datas.flexion[4], 0);
}

void KnuckleDeviceDriver::SetupProps(vr::PropertyContainerHandle_t& props) {
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)2147483647);
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
  vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Identifiable_Bool, true);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_Firmware_RemindUpdate_Bool, false);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_Axis1Type_Int32, vr::k_eControllerAxis_Trigger);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32,
                                       IsRightHand() ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasDisplayComponent_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasCameraComponent_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasDriverDirectModeComponent_Bool, false);
  vr::VRProperties()->SetBoolProperty(props, vr::Prop_HasVirtualDisplayComponent_Bool, false);
  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, IsRightHand() ? "Knuckles Right" : "Knuckles Left");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_RenderModelName_String,
                                        IsRightHand() ? "{indexcontroller}valve_controller_knu_1_0_right" : "{indexcontroller}valve_controller_knu_1_0_left");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceDriverManufacturer);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_TrackingFirmwareVersion_String,
                                        "1562916277 watchman@ValveBuilder02 2019-07-12 FPGA 538(2.26/10/2) BL 0 VRC 1562916277 Radio 1562882729");
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
  vr::VRProperties()->SetStringProperty(props, vr::Prop_RegisteredDeviceType_String,
                                        IsRightHand() ? "valve/index_controllerLHR-E217CD01" : "valve/index_controllerLHR-E217CD00");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, "{indexcontroller}/input/index_controller_profile.json");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceOff_String,
                                        IsRightHand() ? "{openglove}/icons/right_controller_status_off.png" : "{openglove}/icons/left_controller_status_off.png");
  vr::VRProperties()->SetStringProperty(
      props, vr::Prop_NamedIconPathDeviceSearching_String,
      IsRightHand() ? "{openglove}/icons/right_controller_status_searching.gif" : "{openglove}/icons/left_controller_status_searching.gif");
  vr::VRProperties()->SetStringProperty(
      props, vr::Prop_NamedIconPathDeviceSearchingAlert_String,
      IsRightHand() ? "{openglove}/icons/right_controller_status_searching_alert.gif" : "{openglove}/icons/left_controller_status_searching_alert.gif");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceReady_String,
                                        IsRightHand() ? "{openglove}/icons/right_controller_status_ready.png" : "{openglove}/icons/left_controller_status_ready.png");
  vr::VRProperties()->SetStringProperty(
      props, vr::Prop_NamedIconPathDeviceReadyAlert_String,
      IsRightHand() ? "{openglove}/icons/right_controller_status_ready_alert.png" : "{openglove}/icons/left_controller_status_ready_alert.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceNotReady_String,
                                        IsRightHand() ? "{openglove}/icons/right_controller_status_error.png" : "{openglove}/icons/left_controller_status_error.png");
  vr::VRProperties()->SetStringProperty(props, vr::Prop_NamedIconPathDeviceStandby_String,
                                        IsRightHand() ? "{openglove}/icons/right_controller_status_off.png" : "{openglove}/icons/left_controller_status_off.png");
  vr::VRProperties()->SetStringProperty(
      props, vr::Prop_NamedIconPathDeviceAlertLow_String,
      IsRightHand() ? "{openglove}/icons/right_controller_status_ready_low.png" : "{openglove}/icons/left_controller_status_ready_low.png");

  vr::VRProperties()->SetInt32Property(props, vr::Prop_Axis2Type_Int32, vr::k_eControllerAxis_Trigger);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, "knuckles");

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::SYSTEM_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/touch", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::SYSTEM_TOUCH]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRIGGER_CLICK]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trigger/value", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRIGGER_VALUE],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRACKPAD_X],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRACKPAD_Y],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trackpad/touch", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRACKPAD_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/force", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::TRACKPAD_FORCE],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grip/touch", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::GRIP_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/force", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::GRIP_FORCE],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/value", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::GRIP_VALUE],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/click", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/touch", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/x", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_X],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/y", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::THUMBSTICK_Y],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/click", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::A_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/touch", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::A_TOUCH]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/click", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::B_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/touch", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::B_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_INDEX],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_MIDDLE],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_RING],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &m_inputComponentHandles[(int)KnuckleDeviceComponentIndex::FINGER_PINKY],
                                             vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateHapticComponent(props, "/output/haptic", &m_haptic);
}

void KnuckleDeviceDriver::StartingDevice() {
  m_ffbProvider = std::make_unique<FFBListener>(
      [&](VRFFBData data) {
        // Queue the force feedback data for sending.
        m_communicationManager->QueueSend(data);
      },
      m_configuration.role);
  m_ffbProvider->Start();
}

void KnuckleDeviceDriver::StoppingDevice() { m_ffbProvider->Stop(); }
