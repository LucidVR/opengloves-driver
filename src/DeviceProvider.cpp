#include <DeviceProvider.h>
#include <Windows.h>

#include <algorithm>
#include <string>

#include "Communication/BTSerialCommunicationManager.h"
#include "Communication/NamedPipeCommunicationManager.h"
#include "Communication/SerialCommunicationManager.h"
#include "DeviceDriver/KnuckleDriver.h"
#include "DeviceDriver/LucidGloveDriver.h"
#include "DriverLog.h"
#include "Encode/AlphaEncodingManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "Util/Quaternion.h"
#include "Util/Windows.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "?"
#endif

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  if (const vr::EVRInitError initError = InitServerDriverContext(pDriverContext); initError != vr::EVRInitError::VRInitError_None) return initError;

  VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext)
  InitDriverLog(vr::VRDriverLog());

  // this won't print if running in release
  DebugDriverLog("OpenGloves is running in DEBUG mode");

  const std::string driverPath = GetDriverPath();
  DriverLog("Path to DLL: %s", driverPath.c_str());

  const std::string commitHash = GIT_COMMIT_HASH;
  DriverLog("Built from: %s", commitHash.substr(0, 10).c_str());

  // Create background process for the overlay (used for finding controllers to bind to for tracking)
  if (!CreateBackgroundProcess(driverPath + R"(\bin\win64\openglove_overlay.exe)")) {
    DriverLog("Could not create background process: %c", GetLastErrorAsString().c_str());

    return vr::VRInitError_Init_FileNotFound;
  }

  const VRDeviceConfiguration leftConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_LeftHand);
  const VRDeviceConfiguration rightConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_RightHand);

  const auto boneAnimator = std::make_shared<BoneAnimator>(driverPath + R"(\resources\anims\glove_anim.glb)");

  if (leftConfiguration.enabled) {
    leftHand_ = InstantiateDeviceDriver(leftConfiguration, boneAnimator);
    vr::VRServerDriverHost()->TrackedDeviceAdded(leftHand_->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, leftHand_.get());
  }

  if (rightConfiguration.enabled) {
    rightHand_ = InstantiateDeviceDriver(rightConfiguration, boneAnimator);
    vr::VRServerDriverHost()->TrackedDeviceAdded(rightHand_->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, rightHand_.get());
  }

  return vr::VRInitError_None;
}

std::unique_ptr<DeviceDriver> DeviceProvider::InstantiateDeviceDriver(
    VRDeviceConfiguration configuration, std::shared_ptr<BoneAnimator> boneAnimator) const {
  std::unique_ptr<CommunicationManager> communicationManager;
  std::unique_ptr<EncodingManager> encodingManager;

  const bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;

  switch (configuration.encodingProtocol) {
    default:
      DriverLog("No encoding protocol set. Using legacy.");
    case VREncodingProtocol::Legacy: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_legacyEncodingSettingsSection, "max_analog_value");
      encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
      break;
    }
    case VREncodingProtocol::Alpha: {
      const float maxAnalogValue = vr::VRSettings()->GetFloat(c_alphaEncodingSettingsSection, "max_analog_value");
      encodingManager = std::make_unique<AlphaEncodingManager>(maxAnalogValue);
      break;
    }
  }

  switch (configuration.communicationProtocol) {
    case VRCommunicationProtocol::NamedPipe: {
      DriverLog("Communication set to Named Pipe");
      const std::string path = R"(\\.\pipe\vrapplication\input\glove\v1\)" + std::string(isRightHand ? "right" : "left");
      VRNamedPipeInputConfiguration namedPipeConfiguration(path);
      communicationManager = std::make_unique<NamedPipeCommunicationManager>(namedPipeConfiguration, configuration);
      break;
    }
    case VRCommunicationProtocol::BtSerial: {
      DriverLog("Communication set to BTSerial");
      char name[248];
      vr::VRSettings()->GetString(c_btserialCommunicationSettingsSection, isRightHand ? "right_name" : "left_name", name, sizeof name);
      VRBTSerialConfiguration btSerialSettings(name);
      communicationManager = std::make_unique<BTSerialCommunicationManager>(std::move(encodingManager), btSerialSettings, configuration);
      break;
    }
    default:
      DriverLog("No communication protocol set. Using serial.");
    case VRCommunicationProtocol::Serial:
      char port[16];
      vr::VRSettings()->GetString(c_serialCommunicationSettingsSection, isRightHand ? "right_port" : "left_port", port, sizeof port);
      const int baudRate = vr::VRSettings()->GetInt32(c_serialCommunicationSettingsSection, "baud_rate");
      VRSerialConfiguration serialSettings(port, baudRate);

      communicationManager = std::make_unique<SerialCommunicationManager>(std::move(encodingManager), serialSettings, configuration);
      break;
  }

  switch (configuration.deviceDriver) {
    case VRDeviceDriver::EmulatedKnuckles: {
      char serialNumber[32];
      vr::VRSettings()->GetString(
          c_knuckleDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof serialNumber);
      bool approximateThumb = vr::VRSettings()->GetBool(c_knuckleDeviceSettingsSection, "approximate_thumb");
      return std::make_unique<KnuckleDeviceDriver>(
          std::move(communicationManager), std::move(boneAnimator), serialNumber, approximateThumb, configuration);
    }

    default:
      DriverLog("No device driver selected. Using lucidgloves.");
    case VRDeviceDriver::LucidGloves: {
      char serialNumber[32];
      vr::VRSettings()->GetString(
          c_lucidGloveDeviceSettingsSection, isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof serialNumber);

      return std::make_unique<LucidGloveDeviceDriver>(std::move(communicationManager), std::move(boneAnimator), serialNumber, configuration);
    }
  }
}

VRDeviceConfiguration DeviceProvider::GetDeviceConfiguration(const vr::ETrackedControllerRole role) {
  const bool isRightHand = role == vr::TrackedControllerRole_RightHand;

  DriverLog("Getting configuration for: %s", isRightHand ? "Right hand" : "Left hand");

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
  const vr::HmdQuaternion_t angleOffsetQuaternion = EulerToQuaternion(DegToRad(offsetZRot), DegToRad(offsetYRot), DegToRad(offsetXRot));

  return VRDeviceConfiguration(
      role,
      isEnabled,
      feedbackEnabled,
      VRPoseConfiguration(offsetVector, angleOffsetQuaternion, poseTimeOffset, controllerOverrideEnabled, controllerIdOverride, calibrationButton),
      encodingProtocol,
      communicationProtocol,
      deviceDriver);
}

void DeviceProvider::Cleanup() {}

const char* const* DeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame() {
  if (leftHand_ && leftHand_->IsActive()) leftHand_->RunFrame();
  if (rightHand_ && rightHand_->IsActive()) rightHand_->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}
