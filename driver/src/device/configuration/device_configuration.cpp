#include "device_configuration.h"

#include "util/driver_math.h"

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

og::DeviceDefaultConfiguration GetDriverLegacyConfiguration(vr::ETrackedControllerRole role) {
  og::DeviceDefaultConfiguration result{};

  GetAlphaEncodingSettings(role, result.encoding_configuration);

  return result;
}

PoseConfiguration GetPoseConfiguration(vr::ETrackedControllerRole role) {
  PoseConfiguration result{};

  const bool is_right_hand = role == vr::TrackedControllerRole_RightHand;

  const float offsetXPos = vr::VRSettings()->GetFloat(k_pose_settings_section, is_right_hand ? "right_x_offset_position" : "left_x_offset_position");
  const float offsetYPos = vr::VRSettings()->GetFloat(k_pose_settings_section, is_right_hand ? "right_y_offset_position" : "left_y_offset_position");
  const float offsetZPos = vr::VRSettings()->GetFloat(k_pose_settings_section, is_right_hand ? "right_z_offset_position" : "left_z_offset_position");
  result.offset_position = {offsetXPos, offsetYPos, offsetZPos};

  const float offsetXRot = vr::VRSettings()->GetFloat(k_pose_settings_section, is_right_hand ? "right_x_offset_degrees" : "left_x_offset_degrees");
  const float offsetYRot = vr::VRSettings()->GetFloat(k_pose_settings_section, is_right_hand ? "right_y_offset_degrees" : "left_y_offset_degrees");
  const float offsetZRot = vr::VRSettings()->GetFloat(k_pose_settings_section, is_right_hand ? "right_z_offset_degrees" : "left_z_offset_degrees");
  result.offset_orientation = EulerToQuaternion(DEG_TO_RAD(offsetZRot), DEG_TO_RAD(offsetYRot), DEG_TO_RAD(offsetXRot));

  return result;
}

void SetPoseConfiguration(const PoseConfiguration& configuration, vr::ETrackedControllerRole role) {
  bool is_right_hand = role == vr::TrackedControllerRole_RightHand;
  vr::VRSettings()->SetFloat(k_pose_settings_section, is_right_hand ? "right_x_offset_position" : "left_x_offset_position", configuration.offset_position.v[0]);
  vr::VRSettings()->SetFloat(k_pose_settings_section, is_right_hand ? "right_y_offset_position" : "left_y_offset_position", configuration.offset_position.v[1]);
  vr::VRSettings()->SetFloat(k_pose_settings_section, is_right_hand ? "right_z_offset_position" : "left_z_offset_position", configuration.offset_position.v[2]);

  const vr::HmdVector3d_t eulerOffset = QuaternionToEuler(configuration.offset_orientation);
  vr::VRSettings()->SetFloat(k_pose_settings_section, is_right_hand ? "right_x_offset_degrees" : "left_x_offset_degrees", RAD_TO_DEG(eulerOffset.v[2]));
  vr::VRSettings()->SetFloat(k_pose_settings_section, is_right_hand ? "right_y_offset_degrees" : "left_y_offset_degrees", RAD_TO_DEG(eulerOffset.v[1]));
  vr::VRSettings()->SetFloat(k_pose_settings_section, is_right_hand ? "right_z_offset_degrees" : "left_z_offset_degrees", RAD_TO_DEG(eulerOffset.v[0]));

}