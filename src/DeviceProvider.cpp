#include <DeviceProvider.h>
#include <windows.h>

#include <string>

#include "Communication/BTSerialCommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"
#include "DeviceDriver/KnuckleDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DriverLog.h"
#include "Encode/AlphaEncodingManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "Quaternion.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

std::string GetDriverPath() {
  char path[MAX_PATH];
  HMODULE hm = NULL;

  if (GetModuleHandleEx(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          (LPCSTR)&__ImageBase, &hm) == 0) {
    int ret = GetLastError();
    fprintf(stderr, "GetModuleHandle failed, error = %d\n", ret);
    // Return or however you want to handle an error.
  }
  if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
    int ret = GetLastError();
    fprintf(stderr, "GetModuleFileName failed, error = %d\n", ret);
    // Return or however you want to handle an error.
  }

  std::string::size_type pos = std::string(path).find_last_of("\\/");
  return std::string(path).substr(0, pos);
}

bool CreateBackgroundProcess() {
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
  
  VRDeviceConfiguration_t leftConfiguration =
      GetDeviceConfiguration(vr::TrackedControllerRole_LeftHand);
  VRDeviceConfiguration_t rightConfiguration =
      GetDeviceConfiguration(vr::TrackedControllerRole_RightHand);

  if (leftConfiguration.enabled) {
    m_leftHand = InstantiateDeviceDriver(leftConfiguration);
    vr::VRServerDriverHost()->TrackedDeviceAdded(
        m_leftHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_leftHand.get());
  }
  if (rightConfiguration.enabled) {
    m_rightHand = InstantiateDeviceDriver(rightConfiguration);
    vr::VRServerDriverHost()->TrackedDeviceAdded(m_rightHand->GetSerialNumber().c_str(),
                                                 vr::TrackedDeviceClass_Controller,
                                                 m_rightHand.get());
  }

  return vr::VRInitError_None;
}

static bool TryInstantiateDeviceDriver(VRDeviceConfiguration_t configuration, bool withoutPrefix, std::unique_ptr<IDeviceDriver>* driver) {
  vr::EVRSettingsError err = vr::EVRSettingsError::VRSettingsError_None;

  std::unique_ptr<ICommunicationManager> communicationManager;
  std::unique_ptr<IEncodingManager> encodingManager;

  bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;
  switch (configuration.encodingProtocol) {
    default:
      DriverLog("No encoding protocol set. Using legacy.");
    case VREncodingProtocol::LEGACY: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(
        withoutPrefix ? LEGACY_ENCODING_SETTINGS_SECTION_WITHOUT_PREFIX : LEGACY_ENCODING_SETTINGS_SECTION, "max_analog_value", &err);
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;
      encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
      break;
    }
    case VREncodingProtocol::ALPHA: {
      const float maxAnalogValue =
          vr::VRSettings()->GetFloat(
            withoutPrefix ? ALPHA_ENCODING_SETTINGS_SECTION_WITHOUT_PREFIX : ALPHA_ENCODING_SETTINGS_SECTION, "max_analog_value", &err);  //
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;
      encodingManager = std::make_unique<AlphaEncodingManager>(maxAnalogValue);
      break;
    }
  }

  switch (configuration.communicationProtocol) {
    case VRCommunicationProtocol::BTSERIAL: {
      DriverLog("Communication set to BTSerial");
      char name[248];
      vr::VRSettings()->GetString(
        withoutPrefix ? BTSERIAL_COMMUNICATION_SETTINGS_SECTION_WITHOUT_PREFIX : BTSERIAL_COMMUNICATION_SETTINGS_SECTION,
        isRightHand ? "right_name" : "left_name", name, sizeof(name), &err);
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;
      VRBTSerialConfiguration_t btSerialSettings(name);
      communicationManager = std::make_unique<BTSerialCommunicationManager>(
          btSerialSettings, std::move(encodingManager));
      break;
    }
    default:
      DriverLog("No communication protocol set. Using serial.");
    case VRCommunicationProtocol::SERIAL:
      char port[16];
      vr::VRSettings()->GetString(
        withoutPrefix ? SERIAL_COMMUNICATION_SETTINGS_SECTION_WITHOUT_PREFIX : SERIAL_COMMUNICATION_SETTINGS_SECTION,
        isRightHand ? "right_port" : "left_port", port, sizeof(port), &err);
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;
      const int baudRate = vr::VRSettings()->GetInt32(
        withoutPrefix ? SERIAL_COMMUNICATION_SETTINGS_SECTION_WITHOUT_PREFIX : SERIAL_COMMUNICATION_SETTINGS_SECTION, "baud_rate", &err);
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;
      VRSerialConfiguration_t serialSettings(port, baudRate);

      communicationManager =
          std::make_unique<SerialCommunicationManager>(serialSettings, std::move(encodingManager));
      break;
  }

  switch (configuration.deviceDriver) {
    case VRDeviceDriver::EMULATED_KNUCKLES: {
      char serialNumber[32];
      vr::VRSettings()->GetString(withoutPrefix ? KNUCKLE_DEVICE_SETTINGS_SECTION_WITHOUT_PREFIX : KNUCKLE_DEVICE_SETTINGS_SECTION,
                                  isRightHand ? "right_serial_number" : "left_serial_number",
                                  serialNumber, sizeof(serialNumber), &err);
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;

      *driver = std::make_unique<KnuckleDeviceDriver>(configuration, std::move(communicationManager),
                                                      serialNumber);
      return true;
    }

    default:
      DriverLog("No device driver selected. Using lucidgloves.");
    case VRDeviceDriver::LUCIDGLOVES: {
      char serialNumber[32];
      vr::VRSettings()->GetString(withoutPrefix ? LUCIDGLOVE_DEVICE_SETTINGS_SECTION_WITHOUT_PREFIX : LUCIDGLOVE_DEVICE_SETTINGS_SECTION,
                                  isRightHand ? "right_serial_number" : "left_serial_number",
                                  serialNumber, sizeof(serialNumber), &err);
      if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
        return false;

      *driver = std::make_unique<LucidGloveDeviceDriver>(
          configuration, std::move(communicationManager), serialNumber);
      return true;
    }
  }
}
std::unique_ptr<IDeviceDriver> DeviceProvider::InstantiateDeviceDriver(
    VRDeviceConfiguration_t configuration) {
  std::unique_ptr<IDeviceDriver> driver = nullptr;
  if (!TryInstantiateDeviceDriver(configuration, false, &driver))
    TryInstantiateDeviceDriver(configuration, true, &driver);
  return driver;
}
static bool TryGetDeviceConfiguration(vr::ETrackedControllerRole role, bool withoutPrefix, VRDeviceConfiguration_t* config) {
  const char* driverSettingsSection = withoutPrefix ? DRIVER_SETTINGS_SECTION_WITHOUT_PREFIX : DRIVER_SETTINGS_SECTION;
  const char* poseSettingsSection = withoutPrefix ? POSE_SETTINGS_SECTION_WITHOUT_PREFIX : POSE_SETTINGS_SECTION;
  vr::EVRSettingsError err = vr::EVRSettingsError::VRSettingsError_None;

  const bool isRightHand = role == vr::TrackedControllerRole_RightHand;

  const bool isEnabled = vr::VRSettings()->GetBool(driverSettingsSection,
                                                   isRightHand ? "right_enabled" : "left_enabled", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;

  const auto communicationProtocol = (VRCommunicationProtocol)vr::VRSettings()->GetInt32(
      driverSettingsSection, "communication_protocol", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const auto encodingProtocol =
      (VREncodingProtocol)vr::VRSettings()->GetInt32(driverSettingsSection, "encoding_protocol", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const auto deviceDriver =
      (VRDeviceDriver)vr::VRSettings()->GetInt32(driverSettingsSection, "device_driver", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  
  const float poseTimeOffset = vr::VRSettings()->GetFloat(poseSettingsSection, "pose_time_offset", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;

  const float offsetXPos = vr::VRSettings()->GetFloat(
      poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const float offsetYPos = vr::VRSettings()->GetFloat(
      poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const float offsetZPos = vr::VRSettings()->GetFloat(
      poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;

  const float offsetXRot = vr::VRSettings()->GetFloat(
      poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const float offsetYRot = vr::VRSettings()->GetFloat(
      poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const float offsetZRot = vr::VRSettings()->GetFloat(
      poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;

  const bool controllerOverrideEnabled =
      vr::VRSettings()->GetBool(poseSettingsSection, "controller_override", &err);
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;
  const int controllerIdOverride =
      controllerOverrideEnabled
          ? vr::VRSettings()->GetInt32(poseSettingsSection, isRightHand
                                                                  ? "controller_override_right"
                                                                  : "controller_override_left", &err)
          : -1;
  if (!withoutPrefix && err != vr::EVRSettingsError::VRSettingsError_None)
    return false;

  const vr::HmdVector3_t offsetVector = {offsetXPos, offsetYPos, offsetZPos};

  // Convert the rotation to a quaternion
  const vr::HmdQuaternion_t angleOffsetQuaternion =
      EulerToQuaternion(DegToRad(offsetXRot), DegToRad(offsetYRot), DegToRad(offsetZRot));

  *config = VRDeviceConfiguration_t(
      role, isEnabled,
      VRPoseConfiguration_t(offsetVector, angleOffsetQuaternion, poseTimeOffset,
                            controllerOverrideEnabled, controllerIdOverride),
      encodingProtocol, communicationProtocol, deviceDriver);
  return true;
}
VRDeviceConfiguration_t DeviceProvider::GetDeviceConfiguration(vr::ETrackedControllerRole role) {
  VRDeviceConfiguration_t config({}, false, {{}, {}, {}, false, 0}, {}, {}, {});
  if (!TryGetDeviceConfiguration(role, false, &config))
    TryGetDeviceConfiguration(role, true, &config);
  return config;
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
