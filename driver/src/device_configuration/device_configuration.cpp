#include "device_configuration.h"

const char* k_driver_settings_section = "driver_opengloves";
const char* k_pose_settings_section = "pose_settings";
const char* k_serial_communication_settings_section = "communication_serial";
const char* k_btserial_communication_settings_section = "communication_btserial";
const char* k_alpha_encoding_settings_section = "encoding_alpha";

static void GetBluetoothSerialSettings(const vr::ETrackedControllerRole role, og::BluetoothConfiguration& out_configuration) {
  vr::CVRSettingHelper settings_helper(vr::VRSettings());

  out_configuration.name =
      settings_helper.GetString(k_btserial_communication_settings_section, role == vr::TrackedControllerRole_LeftHand ? "left_name" : "right_name");
}

static void GetSerialSettings(const vr::ETrackedControllerRole role, og::SerialConfiguration& out_configuration) {
  vr::CVRSettingHelper settings_helper(vr::VRSettings());

  out_configuration.port_name =
      settings_helper.GetString(k_serial_communication_settings_section, role == vr::TrackedControllerRole_LeftHand ? "left_port" : "right_port");
}

static void GetAlphaEncodingSettings(const vr::ETrackedControllerRole role, og::EncodingConfiguration& out_configuration) {
  vr::CVRSettingHelper settings_helper(vr::VRSettings());

  out_configuration.max_analog_value = settings_helper.GetInt32(k_alpha_encoding_settings_section, "max_analog_value");
}

og::LegacyConfiguration GetDriverLegacyConfiguration(const vr::ETrackedControllerRole role) {
  og::LegacyConfiguration result;

  result.hand = role == vr::TrackedControllerRole_LeftHand ? og::kHandLeft : og::kHandRight;

  GetBluetoothSerialSettings(role, result.bluetooth_configuration);
  GetSerialSettings(role, result.serial_configuration);
  GetAlphaEncodingSettings(role, result.encoding_configuration);

  return result;
}