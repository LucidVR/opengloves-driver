#include "DeviceConfiguration.h"

#include "Util/Quaternion.h"

const char* c_poseSettingsSection = "pose_settings";
const char* c_driverSettingsSection = "driver_openglove";
const char* c_serialCommunicationSettingsSection = "communication_serial";
const char* c_btserialCommunicationSettingsSection = "communication_btserial";
const char* c_knuckleDeviceSettingsSection = "device_knuckles";
const char* c_lucidGloveDeviceSettingsSection = "device_lucidgloves";
const char* c_alphaEncodingSettingsSection = "encoding_alpha";
const char* c_legacyEncodingSettingsSection = "encoding_legacy";

const char* c_deviceDriverManufacturer = "LucidVR";

VRDeviceConfiguration GetDeviceConfiguration(const vr::ETrackedControllerRole role) {
  const bool isRightHand = role == vr::TrackedControllerRole_RightHand;

  const bool isEnabled = vr::VRSettings()->GetBool(c_driverSettingsSection, isRightHand ? "right_enabled" : "left_enabled");
  const bool feedbackEnabled = vr::VRSettings()->GetBool(c_driverSettingsSection, "feedback_enabled");

  const auto communicationProtocol =
      static_cast<VRCommunicationProtocol>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "communication_protocol"));
  const auto encodingProtocol = static_cast<VREncodingProtocol>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "encoding_protocol"));
  const auto deviceDriver = static_cast<VRDeviceDriver>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "device_driver"));

  const float poseTimeOffset = vr::VRSettings()->GetFloat(c_poseSettingsSection, "pose_time_offset");

  const float offsetXPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_position" : "left_x_offset_position");
  const float offsetYPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_position" : "left_y_offset_position");
  const float offsetZPos = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_position" : "left_z_offset_position");

  const float offsetXRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_x_offset_degrees" : "left_x_offset_degrees");
  const float offsetYRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_y_offset_degrees" : "left_y_offset_degrees");
  const float offsetZRot = vr::VRSettings()->GetFloat(c_poseSettingsSection, isRightHand ? "right_z_offset_degrees" : "left_z_offset_degrees");

  const bool controllerOverrideEnabled = vr::VRSettings()->GetBool(c_poseSettingsSection, "controller_override");
  const int controllerIdOverride =
      controllerOverrideEnabled
          ? vr::VRSettings()->GetInt32(c_poseSettingsSection, isRightHand ? "controller_override_right" : "controller_override_left")
          : -1;
  const bool calibrationButton = vr::VRSettings()->GetBool(c_poseSettingsSection, "hardware_calibration_button_enabled");

  const vr::HmdVector3_t offsetVector = {offsetXPos, offsetYPos, offsetZPos};

  // Convert the rotation to a quaternion
  const vr::HmdQuaternion_t angleOffsetQuaternion = EulerToQuaternion(DegToRad(offsetXRot), DegToRad(offsetYRot), DegToRad(offsetZRot));

  return VRDeviceConfiguration(
      role,
      isEnabled,
      feedbackEnabled,
      VRPoseConfiguration(offsetVector, angleOffsetQuaternion, poseTimeOffset, controllerOverrideEnabled, controllerIdOverride, calibrationButton),
      encodingProtocol,
      communicationProtocol,
      deviceDriver);
}
