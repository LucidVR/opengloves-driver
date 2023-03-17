// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include "nlohmann/json.hpp"

#include <variant>

#include "openvr_driver.h"

extern const char* k_driver_settings_section;
extern const char* k_pose_settings_section;

extern const char* k_serial_communication_settings_section;
extern const char* k_btserial_communication_settings_section;
extern const char* k_namedpipe_communication_settings_section;
extern const char* k_alpha_encoding_settings_section;

struct PoseConfiguration {
  vr::HmdQuaternion_t offset_orientation;
  vr::HmdVector3d_t offset_position;
};

nlohmann::ordered_map<std::string, std::variant<bool>> GetDriverConfigurationMap();
nlohmann::ordered_map<std::string, std::variant<bool, std::string>> GetBluetoothSerialConfigurationMap();
nlohmann::ordered_map<std::string, std::variant<bool, std::string>> GetSerialConfigurationMap();
nlohmann::ordered_map<std::string, std::variant<bool>> GetNamedPipeConfigurationMap();

nlohmann::ordered_map<std::string, std::variant<int>> GetAlphaEncodingConfigurationMap();
nlohmann::ordered_map<std::string, std::variant<float, bool>> GetPoseConfigurationMap();

PoseConfiguration GetPoseConfiguration(vr::ETrackedControllerRole role);
void SetPoseConfiguration(const PoseConfiguration& configuration, vr::ETrackedControllerRole role);