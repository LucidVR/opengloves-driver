#include <DeviceProvider.h>
#include <windows.h>

#include <algorithm>
#include <string>

#include "Communication/BTSerialCommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"
#include "DeviceDriver/KnuckleDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DriverLog.h"
#include "Encode/AlphaEncodingManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "Quaternion.h"
#include "Util/Windows.h"

static bool CreateBackgroundProcess() {
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  const std::string driverPath = GetDriverPath();
  DriverLog("Path to DLL: %s", driverPath.c_str());

  std::string path = driverPath + "\\openglove_overlay.exe";

  bool success = true;
  if (!CreateProcess(path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) success = false;

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success;
}

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
  if (initError != vr::EVRInitError::VRInitError_None) return initError;

  VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
  InitDriverLog(vr::VRDriverLog());
  DebugDriverLog("OpenGlove is running in DEBUG mode");

  if (!CreateBackgroundProcess()) {
    DriverLog("Could not create background process");
    return vr::VRInitError_Init_FileNotFound;
  }

  VRDeviceConfiguration_t leftConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_LeftHand);
  VRDeviceConfiguration_t rightConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_RightHand);

  std::string driverPath = GetDriverPath();

  const std::string unwanted = "\\bin\\win64";
  driverPath.erase(driverPath.find(unwanted), unwanted.length());

  std::shared_ptr<BoneAnimator> boneAnimator = std::make_shared<BoneAnimator>(driverPath + "\\resources\\anims\\glove_anim.glb");

  if (leftConfiguration.enabled) {
    m_leftHand = InstantiateDeviceDriver(leftConfiguration, boneAnimator);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_leftHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_leftHand.get());
  }
  if (rightConfiguration.enabled) {
    m_rightHand = InstantiateDeviceDriver(rightConfiguration, boneAnimator);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_rightHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_rightHand.get());
  }

  return vr::VRInitError_None;
}

std::unique_ptr<DeviceDriver> DeviceProvider::InstantiateDeviceDriver(VRDeviceConfiguration_t configuration, std::shared_ptr<BoneAnimator> boneAnimator) {
  std::unique_ptr<CommunicationManager> communicationManager;
  std::unique_ptr<EncodingManager> encodingManager;

  bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;
  switch (configuration.encodingProtocol) {
    default:
      DriverLog("No encoding protocol set. Using legacy.");
    case VREncodingProtocol::LEGACY: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_legacyEncodingSettingsSection, "max_analog_value");
      encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
      break;
    }
    case VREncodingProtocol::ALPHA: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_alphaEncodingSettingsSection, "max_analog_value");
      encodingManager = std::make_unique<AlphaEncodingManager>(maxAnalogValue);
      break;
    }
  }

  switch (configuration.communicationProtocol) {
    case VRCommunicationProtocol::BTSERIAL: {
      DriverLog("Communication set to BTSerial");
      char name[248];
      vr::VRSettings()->GetString(c_btserialCommunicationSettingsSection, isRightHand ? "right_name" : "left_name", name, sizeof(name));
      VRBTSerialConfiguration_t btSerialSettings(name);
      communicationManager = std::make_unique<BTSerialCommunicationManager>(std::move(encodingManager), btSerialSettings, configuration);
      break;
    }
    default:
      DriverLog("No communication protocol set. Using serial.");
    case VRCommunicationProtocol::SERIAL:
      char port[16];
      vr::VRSettings()->GetString(c_serialCommunicationSettingsSection, isRightHand ? "right_port" : "left_port", port, sizeof(port));
      const int baudRate = vr::VRSettings()->GetInt32(c_serialCommunicationSettingsSection, "baud_rate");
      VRSerialConfiguration_t serialSettings(port, baudRate);

      communicationManager = std::make_unique<SerialCommunicationManager>(std::move(encodingManager), serialSettings, configuration);
      break;
  }

  switch (configuration.deviceDriver) {
    case VRDeviceDriver::EMULATED_KNUCKLES: {
      char serialNumber[32];
      vr::VRSettings()->GetString(c_knuckleDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof(serialNumber));

      return std::make_unique<KnuckleDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration);
    }

    default:
      DriverLog("No device driver selected. Using lucidgloves.");
    case VRDeviceDriver::LUCIDGLOVES: {
      char serialNumber[32];
      vr::VRSettings()->GetString(c_lucidGloveDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof(serialNumber));

      return std::make_unique<LucidGloveDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration);
    }
  }
}

VRDeviceConfiguration_t DeviceProvider::GetDeviceConfiguration(vr::ETrackedControllerRole role) {
  const bool isRightHand = role == vr::TrackedControllerRole_RightHand;

  const bool isEnabled = vr::VRSettings()->GetBool(c_driverSettingsSection, isRightHand ? "right_enabled" : "left_enabled");
  const bool feedbackEnabled = vr::VRSettings()->GetBool(c_driverSettingsSection, "feedback_enabled");

  const auto communicationProtocol = (VRCommunicationProtocol)vr::VRSettings()->GetInt32(c_driverSettingsSection, "communication_protocol");
  const auto encodingProtocol = (VREncodingProtocol)vr::VRSettings()->GetInt32(c_driverSettingsSection, "encoding_protocol");
  const auto deviceDriver = (VRDeviceDriver)vr::VRSettings()->GetInt32(c_driverSettingsSection, "device_driver");

  const float poseTimeOffset = vr::VRSettings()->GetFloat(c_poseSettingsSection, "pose_time_offset");

  const float offsetXPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position");
  const float offsetYPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position");
  const float offsetZPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position");

  const float offsetXRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees");
  const float offsetYRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees");
  const float offsetZRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees");

  const bool controllerOverrideEnabled = vr::VRSettings()->GetBool(c_poseSettingsSection, "controller_override");
  const int controllerIdOverride =
      controllerOverrideEnabled ? vr::VRSettings()->GetInt32(c_poseSettingsSection, isRightHand ? "controller_override_right" : "controller_override_left") : -1;
  const bool calibrationButton = vr::VRSettings()->GetBool(c_poseSettingsSection, "hardware_calibration_button_enabled");

  const vr::HmdVector3_t offsetVector = {offsetXPos, offsetYPos, offsetZPos};

  // Convert the rotation to a quaternion
  const vr::HmdQuaternion_t angleOffsetQuaternion = EulerToQuaternion(DegToRad(offsetXRot), DegToRad(offsetYRot), DegToRad(offsetZRot));

  return VRDeviceConfiguration_t(
    role,
    isEnabled,
    feedbackEnabled,
    VRPoseConfiguration_t(
      offsetVector,
      angleOffsetQuaternion,
      poseTimeOffset,
      controllerOverrideEnabled,
      controllerIdOverride,
      calibrationButton),
    encodingProtocol,
    communicationProtocol,
    deviceDriver);
}

void DeviceProvider::Cleanup() {}

const char* const* DeviceProvider::GetInterfaceVersions() { return vr::k_InterfaceVersions; }

void DeviceProvider::RunFrame() {
  if (m_leftHand && m_leftHand->IsActive()) m_leftHand->RunFrame();
  if (m_rightHand && m_rightHand->IsActive()) m_rightHand->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode() { return false; }

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}
