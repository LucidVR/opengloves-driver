#pragma once

#include "opengloves_interface.h"
#include "openvr_driver.h"

extern const char* k_driver_settings_section;
extern const char* k_pose_settings_section;
extern const char* k_serial_communication_settings_section;
extern const char* k_btserial_communication_settings_section;
extern const char* k_alpha_encoding_settings_section;

struct PoseConfiguration {
  vr::HmdQuaternion_t offset_orientation;
  vr::HmdVector3d_t offset_position;
};

og::DeviceDefaultConfiguration GetDriverLegacyConfiguration(vr::ETrackedControllerRole role);

PoseConfiguration GetPoseConfiguration(vr::ETrackedControllerRole role);
void SetPoseConfiguration(const PoseConfiguration& configuration, vr::ETrackedControllerRole role);