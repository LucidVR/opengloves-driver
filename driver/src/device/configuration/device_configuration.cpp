// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "device_configuration.h"

#include "util/driver_math.h"

#include "util/driver_log.h"

const char* k_driver_settings_section = "driver_opengloves";

const char* k_device_lucidgloves_section = "device_lucidgloves";
const char* k_device_knuckles_section = "device_knuckles";

const char* k_pose_settings_section = "pose_settings";

const char* k_serial_communication_settings_section = "communication_serial";
const char* k_btserial_communication_settings_section = "communication_btserial";
const char* k_namedpipe_communication_settings_section = "communication_namedpipe";
const char* k_alpha_encoding_settings_section = "encoding_alpha";

nlohmann::ordered_map<std::string, std::variant<bool>> GetDriverConfigurationMap() {
  nlohmann::ordered_map<std::string, std::variant<bool>> result{};

  result["enable"] = vr::VRSettings()->GetBool(k_driver_settings_section, "enable");

  result["left_enabled"] = vr::VRSettings()->GetBool(k_driver_settings_section, "left_enabled");
  result["right_enabled"] = vr::VRSettings()->GetBool(k_driver_settings_section, "right_enabled");
  result["feedback_enabled"] = vr::VRSettings()->GetBool(k_driver_settings_section, "feedback_enabled");

  return result;
}

nlohmann::ordered_map<std::string, std::variant<bool, std::string>> GetBluetoothSerialConfigurationMap() {
  vr::CVRSettingHelper settings_helper(vr::VRSettings());

  nlohmann::ordered_map<std::string, std::variant<bool, std::string>> result{};

  result["enabled"] = vr::VRSettings()->GetBool(k_btserial_communication_settings_section, "enabled");

  result["left_name"] = settings_helper.GetString(k_btserial_communication_settings_section, "left_name");
  result["right_name"] = settings_helper.GetString(k_btserial_communication_settings_section, "right_name");

  return result;
}

nlohmann::ordered_map<std::string, std::variant<bool, std::string>> GetSerialConfigurationMap() {
  vr::CVRSettingHelper settings_helper(vr::VRSettings());

  nlohmann::ordered_map<std::string, std::variant<bool, std::string>> result{};

  result["enabled"] = vr::VRSettings()->GetBool(k_serial_communication_settings_section, "enabled");

  result["left_port"] = settings_helper.GetString(k_serial_communication_settings_section, "left_port");
  result["right_port"] = settings_helper.GetString(k_serial_communication_settings_section, "right_port");

  return result;
}

nlohmann::ordered_map<std::string, std::variant<bool>> GetNamedPipeConfigurationMap() {
  nlohmann::ordered_map<std::string, std::variant<bool>> result{};
  result["enabled"] = vr::VRSettings()->GetBool(k_namedpipe_communication_settings_section, "enabled");

  return result;
}

nlohmann::ordered_map<std::string, std::variant<int>> GetAlphaEncodingConfigurationMap() {
  nlohmann::ordered_map<std::string, std::variant<int>> result{};

  result["max_analog_value"] = vr::VRSettings()->GetInt32(k_alpha_encoding_settings_section, "max_analog_value");

  if (std::get<int>(result["max_analog_value"]) == 0){
    DriverLog("max_analog_value is set to zero. This will cause errors, and your glove probably won't work.");
    result["max_analog_value"] = 1;
  }
  return result;
}

nlohmann::ordered_map<std::string, std::variant<float, bool>> GetPoseConfigurationMap() {
  nlohmann::ordered_map<std::string, std::variant<float, bool>> result{};

  result["right_x_offset_position"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "right_x_offset_position");
  result["right_y_offset_position"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "right_y_offset_position");
  result["right_z_offset_position"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "right_z_offset_position");
  result["right_x_offset_degrees"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "right_x_offset_degrees");
  result["right_y_offset_degrees"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "right_y_offset_degrees");
  result["right_z_offset_degrees"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "right_z_offset_degrees");

  result["left_x_offset_position"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "left_x_offset_position");
  result["left_y_offset_position"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "left_y_offset_position");
  result["left_z_offset_position"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "left_z_offset_position");
  result["left_x_offset_degrees"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "left_x_offset_degrees");
  result["left_y_offset_degrees"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "left_y_offset_degrees");
  result["left_z_offset_degrees"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "left_z_offset_degrees");

  result["pose_time_offset"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "pose_time_offset");

  result["controller_override"] = vr::VRSettings()->GetBool(k_pose_settings_section, "controller_override");
  result["controller_override_left"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "controller_override_left");
  result["controller_override_right"] = vr::VRSettings()->GetFloat(k_pose_settings_section, "controller_override_right");

  return result;
}

PoseConfiguration GetPoseConfiguration(vr::ETrackedControllerRole role) {
  PoseConfiguration result{};

  const bool is_right_hand = role == vr::TrackedControllerRole_RightHand;
  nlohmann::ordered_map<std::string, std::variant<float, bool>> pose_configuration_map = GetPoseConfigurationMap();

  result.offset_position = {
      std::get<float>(pose_configuration_map.at(is_right_hand ? "right_x_offset_position" : "left_x_offset_position")),
      std::get<float>(pose_configuration_map.at(is_right_hand ? "right_y_offset_position" : "left_y_offset_position")),
      std::get<float>(pose_configuration_map.at(is_right_hand ? "right_z_offset_position" : "left_z_offset_position"))};

  result.offset_orientation = EulerToQuaternion(
      DEG_TO_RAD(std::get<float>(pose_configuration_map.at(is_right_hand ? "right_z_offset_degrees" : "left_z_offset_degrees"))),
      DEG_TO_RAD(std::get<float>(pose_configuration_map.at(is_right_hand ? "right_y_offset_degrees" : "left_y_offset_degrees"))),
      DEG_TO_RAD(std::get<float>(pose_configuration_map.at(is_right_hand ? "right_x_offset_degrees" : "left_x_offset_degrees"))));

  return result;
}

void SetPoseConfiguration(const PoseConfiguration& configuration, vr::ETrackedControllerRole role) {
  bool is_right_hand = role == vr::TrackedControllerRole_RightHand;
  vr::VRSettings()->SetFloat(
      k_pose_settings_section, is_right_hand ? "right_x_offset_position" : "left_x_offset_position", configuration.offset_position.v[0]);
  vr::VRSettings()->SetFloat(
      k_pose_settings_section, is_right_hand ? "right_y_offset_position" : "left_y_offset_position", configuration.offset_position.v[1]);
  vr::VRSettings()->SetFloat(
      k_pose_settings_section, is_right_hand ? "right_z_offset_position" : "left_z_offset_position", configuration.offset_position.v[2]);

  const vr::HmdVector3d_t eulerOffset = QuaternionToEuler(configuration.offset_orientation);
  vr::VRSettings()->SetFloat(
      k_pose_settings_section, is_right_hand ? "right_x_offset_degrees" : "left_x_offset_degrees", RAD_TO_DEG(eulerOffset.v[2]));
  vr::VRSettings()->SetFloat(
      k_pose_settings_section, is_right_hand ? "right_y_offset_degrees" : "left_y_offset_degrees", RAD_TO_DEG(eulerOffset.v[1]));
  vr::VRSettings()->SetFloat(
      k_pose_settings_section, is_right_hand ? "right_z_offset_degrees" : "left_z_offset_degrees", RAD_TO_DEG(eulerOffset.v[0]));
}