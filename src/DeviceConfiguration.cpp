#include "DeviceConfiguration.h"

#include <map>

#include "DriverLog.h"
#include "Util/Quaternion.h"

const char* c_poseSettingsSection = "pose_settings";
const char* c_driverSettingsSection = "driver_openglove";
const char* c_serialCommunicationSettingsSection = "communication_serial";
const char* c_btserialCommunicationSettingsSection = "communication_btserial";
const char* c_knuckleDeviceSettingsSection = "device_knuckles";
const char* c_lucidGloveDeviceSettingsSection = "device_lucidgloves";
const char* c_alphaEncodingSettingsSection = "encoding_alpha";
const char* c_legacyEncodingSettingsSection = "encoding_legacy";

const char* c_deviceManufacturer = "LucidVR";

VRCommunicationConfiguration GetCommunicationConfiguration(const bool isRightHand) {
  vr::CVRSettingHelper settingHelper(vr::VRSettings());

  switch (static_cast<VRCommunicationProtocol>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "communication_protocol"))) {
    case VRCommunicationProtocol::NamedPipe: {
      const std::string pipeName = R"(\\.\pipe\vrapplication\input\glove\$version\)" + std::string(isRightHand ? "right" : "left");

      return {VRCommunicationProtocol::NamedPipe, VRCommunicationNamedPipeConfiguration{pipeName}};
    }

    case VRCommunicationProtocol::BtSerial: {
      const std::string name = settingHelper.GetString(c_btserialCommunicationSettingsSection, isRightHand ? "right_name" : "left_name");

      return {VRCommunicationProtocol::BtSerial, VRCommunicationBTSerialConfiguration{name}};
    }

    default:
      DriverLog("No communication protocol specified. Configuring for serial");
    case VRCommunicationProtocol::Serial: {
      const std::string port = settingHelper.GetString(c_serialCommunicationSettingsSection, isRightHand ? "right_port" : "left_port");
      const int baudRate = vr::VRSettings()->GetInt32(c_serialCommunicationSettingsSection, "baud_rate");

      return {VRCommunicationProtocol::Serial, VRCommunicationSerialConfiguration{port, baudRate}};
    }
  }
}

VRDeviceConfiguration GetDeviceConfiguration(const bool isRightHand) {
  vr::CVRSettingHelper settingHelper(vr::VRSettings());

  switch (static_cast<VRDeviceType>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "device_driver"))) {
    case VRDeviceType::LucidGloves: {
      const std::string serialNumber =
          settingHelper.GetString(c_lucidGloveDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number");

      return {VRDeviceType::LucidGloves, VRDeviceLucidglovesConfiguration{serialNumber}};
    }
    default:
      DriverLog("No device type specified. Configuring for emulated knuckles");
    case VRDeviceType::EmulatedKnuckles: {
      const std::string serialNumber =
          settingHelper.GetString(c_knuckleDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number");

      const bool indexCurlAsTrigger = vr::VRSettings()->GetBool(c_knuckleDeviceSettingsSection, "index_curl_as_trigger");
      const bool approximateThumb = vr::VRSettings()->GetBool(c_knuckleDeviceSettingsSection, "approximate_thumb");

      return {VRDeviceType::EmulatedKnuckles, VRDeviceKnucklesConfiguration{indexCurlAsTrigger, approximateThumb, serialNumber}};
    }
  }
}

VREncodingConfiguration GetEncodingConfiguration() {
  switch (static_cast<VREncodingProtocol>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "encoding_protocol"))) {
    case VREncodingProtocol::Legacy: {
      const unsigned int maxAnalogueValue = vr::VRSettings()->GetInt32(c_legacyEncodingSettingsSection, "max_analog_value");

      return {VREncodingProtocol::Legacy, VRLegacyEncodingConfiguration{maxAnalogueValue}};
    }

    default:
      DriverLog("No encoding protocol specified. Configuring from alpha encoding");
    case VREncodingProtocol::Alpha: {
      const unsigned int maxAnalogueValue = vr::VRSettings()->GetInt32(c_alphaEncodingSettingsSection, "max_analog_value");

      return {VREncodingProtocol::Alpha, VRAlphaEncodingConfiguration{maxAnalogueValue}};
    }
  }
}

VRPoseConfiguration GetPoseConfiguration(bool isRightHand, vr::CVRSettingHelper& settingHelper) {
  const float poseTimeOffset = vr::VRSettings()->GetFloat(c_poseSettingsSection, "pose_time_offset");

  const float offsetXPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position");
  const float offsetYPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position");
  const float offsetZPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position");
  const vr::HmdVector3d_t offsetVector = {offsetXPos, offsetYPos, offsetZPos};

  const float offsetXRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees");
  const float offsetYRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees");
  const float offsetZRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees");
  const vr::HmdQuaternion_t angleOffsetQuaternion = EulerToQuaternion(DegToRad(offsetZRot), DegToRad(offsetYRot), DegToRad(offsetXRot));

  const bool controllerOverrideEnabled = vr::VRSettings()->GetBool(c_poseSettingsSection, "controller_override");
  const int controllerIdOverride =
      controllerOverrideEnabled
          ? vr::VRSettings()->GetInt32(c_poseSettingsSection, isRightHand ? "controller_override_right" : "controller_override_left")
          : -1;
  const bool calibrationButtonEnabled = vr::VRSettings()->GetBool(c_poseSettingsSection, "hardware_calibration_button_enabled");

  return {offsetVector, angleOffsetQuaternion, poseTimeOffset, controllerIdOverride, controllerOverrideEnabled, calibrationButtonEnabled};
}

VRDriverConfiguration GetDriverConfiguration(const vr::ETrackedControllerRole& role) {
  const bool isRightHand = role == vr::TrackedControllerRole_RightHand;
  vr::CVRSettingHelper settingHelper(vr::VRSettings());

  DriverLog("Getting configuration for: %s", isRightHand ? "Right hand" : "Left hand");

  // main settings
  const bool enabled = vr::VRSettings()->GetBool(c_driverSettingsSection, isRightHand ? "right_enabled" : "left_enabled");
  const bool feedbackEnabled = vr::VRSettings()->GetBool(c_driverSettingsSection, "feedback_enabled");

  return {
      enabled,
      role,
      feedbackEnabled,
      GetDeviceConfiguration(isRightHand),
      GetEncodingConfiguration(),
      GetCommunicationConfiguration(isRightHand),
      GetPoseConfiguration(isRightHand, settingHelper)};
}
