#include "DeviceDriver/KnuckleDriver.h"

#include <utility>

#include "DriverLog.h"

namespace knuckleDevice {
	const char* c_deviceManufacturer = "LucasVRTech&Danwillm";
}

enum ComponentIndex : int {
  SYSTEM_CLICK,
  SYSTEM_TOUCH,
  TRIGGER_CLICK,
  TRIGGER_VALUE,
  TRACKPAD_X,
  TRACKPAD_Y,
  TRACKPAD_TOUCH,
  TRACKPAD_FORCE,
  GRIP_TOUCH,
  GRIP_FORCE,
  GRIP_VALUE,
  THUMBSTICK_CLICK,
  THUMBSTICK_TOUCH,
  THUMBSTICK_X,
  THUMBSTICK_Y,
  A_CLICK,
  A_TOUCH,
  B_CLICK,
  B_TOUCH,
  FINGER_INDEX,
  FINGER_MIDDLE,
  FINGER_RING,
  FINGER_PINKY
};

KnuckleDeviceDriver::KnuckleDeviceDriver(VRDeviceConfiguration_t configuration, std::unique_ptr<ICommunicationManager> communicationManager, std::string serialNumber)
    : m_configuration(configuration),
      m_communicationManager(std::move(communicationManager)),
      m_serialNumber(std::move(serialNumber)),
      m_driverId(-1),
      m_hasActivated(false) {
  // copy a default bone transform to our hand transform for use in finger positioning later
  std::copy(std::begin(m_configuration.role == vr::TrackedControllerRole_RightHand ? rightOpenPose : leftOpenPose),
            std::end(m_configuration.role == vr::TrackedControllerRole_RightHand ? rightOpenPose : leftOpenPose), std::begin(m_handTransforms));
}

bool KnuckleDeviceDriver::IsRightHand() const { return m_configuration.role == vr::TrackedControllerRole_RightHand; }

std::string KnuckleDeviceDriver::GetSerialNumber() { return m_serialNumber; }

bool KnuckleDeviceDriver::IsActive() { return m_hasActivated; }

vr::EVRInitError KnuckleDeviceDriver::Activate(uint32_t unObjectId) {
  const bool isRightHand = IsRightHand();
  m_driverId = unObjectId;  // unique ID for your driver
  m_controllerPose = std::make_unique<ControllerPose>(m_configuration.role, std::string(knuckleDevice::c_deviceManufacturer), m_configuration.poseConfiguration);

  vr::PropertyContainerHandle_t props =
      vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId);  // this gets a container object where you store all the information about your driver

  vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)2147483647);
  vr::VRProperties()->SetStringProperty(props, vr::Prop_TrackingSystemName_String, "lighthouse");
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
  vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, knuckleDevice::c_deviceManufacturer);
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

  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/click", &m_inputComponentHandles[ComponentIndex::SYSTEM_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/system/touch", &m_inputComponentHandles[ComponentIndex::SYSTEM_TOUCH]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &m_inputComponentHandles[ComponentIndex::TRIGGER_CLICK]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trigger/value", &m_inputComponentHandles[ComponentIndex::TRIGGER_VALUE], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/x", &m_inputComponentHandles[ComponentIndex::TRACKPAD_X], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/y", &m_inputComponentHandles[ComponentIndex::TRACKPAD_Y], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trackpad/touch", &m_inputComponentHandles[ComponentIndex::TRACKPAD_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/trackpad/force", &m_inputComponentHandles[ComponentIndex::TRACKPAD_FORCE], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grip/touch", &m_inputComponentHandles[ComponentIndex::GRIP_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/force", &m_inputComponentHandles[ComponentIndex::GRIP_FORCE], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/grip/value", &m_inputComponentHandles[ComponentIndex::GRIP_VALUE], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/click", &m_inputComponentHandles[ComponentIndex::THUMBSTICK_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/thumbstick/touch", &m_inputComponentHandles[ComponentIndex::THUMBSTICK_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/x", &m_inputComponentHandles[ComponentIndex::THUMBSTICK_X], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/thumbstick/y", &m_inputComponentHandles[ComponentIndex::THUMBSTICK_Y], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedTwoSided);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/click", &m_inputComponentHandles[ComponentIndex::A_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/a/touch", &m_inputComponentHandles[ComponentIndex::A_TOUCH]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/click", &m_inputComponentHandles[ComponentIndex::B_CLICK]);
  vr::VRDriverInput()->CreateBooleanComponent(props, "/input/b/touch", &m_inputComponentHandles[ComponentIndex::B_TOUCH]);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &m_inputComponentHandles[ComponentIndex::FINGER_INDEX], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &m_inputComponentHandles[ComponentIndex::FINGER_MIDDLE], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &m_inputComponentHandles[ComponentIndex::FINGER_RING], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &m_inputComponentHandles[ComponentIndex::FINGER_PINKY], vr::VRScalarType_Absolute,
                                             vr::VRScalarUnits_NormalizedOneSided);
  vr::VRDriverInput()->CreateHapticComponent(props, "/output/haptic", &m_haptic);

  vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(
      props, isRightHand ? "/input/skeleton/right" : "/input/skeleton/left", isRightHand ? "/skeleton/hand/right" : "/skeleton/hand/left", "/pose/raw",
      vr::VRSkeletalTracking_Partial, isRightHand ? rightOpenPose : leftOpenPose, NUM_BONES, &m_skeletalComponentHandle);

  if (error != vr::VRInputError_None) {
    DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
  }

  StartDevice();

  m_hasActivated = true;

  return vr::VRInitError_None;
}

// This could do with a rename, its a bit vague as to what it does
void KnuckleDeviceDriver::StartDevice() {
  m_ffbProvider = std::make_unique<FFBPipe>();
  m_ffbProvider->Start(
      [&](VRFFBData_t data) {
        // Queue the force feedback data for sending.
        m_communicationManager->QueueSend(data);
      },
      m_configuration.role);
  m_communicationManager->Connect();
  if (m_communicationManager->IsConnected()) {
    m_communicationManager->BeginListener([&](VRCommData_t datas) {
      try {
        // Compute each finger transform
        for (int i = 0; i < NUM_BONES; i++) {
          int fingerNum = FingerFromBone(i);
          if (fingerNum != -1) {
            ComputeBoneFlexion(&m_handTransforms[i], datas.flexion[fingerNum], i, IsRightHand());
          }
        }
        vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
        vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, m_handTransforms, NUM_BONES);

        vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::THUMBSTICK_X], datas.joyX, 0);
        vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::THUMBSTICK_Y], datas.joyY, 0);

        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::THUMBSTICK_CLICK], datas.joyButton, 0);
        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::THUMBSTICK_TOUCH], datas.joyButton, 0);

        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::TRIGGER_CLICK], datas.trgButton, 0);
        vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::TRIGGER_VALUE], datas.flexion[1], 0);

        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::A_CLICK], datas.aButton, 0);
        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::A_TOUCH], datas.aButton, 0);

        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::B_CLICK], datas.bButton, 0);
        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::B_TOUCH], datas.bButton, 0);

        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::GRIP_FORCE], datas.grab, 0);
        vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::GRIP_TOUCH], datas.grab, 0);
        vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::GRIP_VALUE], datas.grab, 0);

				//We don't have a thumb on the index
				//vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::THUMB], datas.flexion[0], 0);
				vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::FINGER_INDEX], datas.flexion[1], 0);
				vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::FINGER_MIDDLE], datas.flexion[2], 0);
				vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::FINGER_RING], datas.flexion[3], 0);
				vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::FINGER_PINKY], datas.flexion[4], 0);
			
				if (datas.calibrate) {
					if (!m_controllerPose->isCalibrating())
						m_controllerPose->StartCalibration();
				}
				else
				{
					if (m_controllerPose->isCalibrating())
						m_controllerPose->FinishCalibration();
				}
			}
			catch (const std::exception& e) {
				DebugDriverLog("Exception caught while parsing comm data");
			}
		});

  } else {
    DebugDriverLog("Device did not connect successfully");
  }
}

vr::DriverPose_t KnuckleDeviceDriver::GetPose() {
  if (m_hasActivated) return m_controllerPose->UpdatePose();

  vr::DriverPose_t pose = {0};
  return pose;
}

void KnuckleDeviceDriver::RunFrame() {
  if (m_hasActivated) {
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_driverId, m_controllerPose->UpdatePose(), sizeof(vr::DriverPose_t));
  }
}

void KnuckleDeviceDriver::Deactivate() {
  if (m_hasActivated) {
    m_communicationManager->Disconnect();
    m_ffbProvider->Stop();
    m_driverId = vr::k_unTrackedDeviceIndexInvalid;
  }
}

void* KnuckleDeviceDriver::GetComponent(const char* pchComponentNameAndVersion) { return nullptr; }

void KnuckleDeviceDriver::EnterStandby() {}

void KnuckleDeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) {
    pchResponseBuffer[0] = 0;
  }
}