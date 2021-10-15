#pragma once

#include "openvr_driver.h"

extern const char* c_poseSettingsSection;
extern const char* c_driverSettingsSection;
extern const char* c_serialCommunicationSettingsSection;
extern const char* c_btserialCommunicationSettingsSection;
extern const char* c_knuckleDeviceSettingsSection;
extern const char* c_lucidGloveDeviceSettingsSection;
extern const char* c_alphaEncodingSettingsSection;
extern const char* c_legacyEncodingSettingsSection;

extern const char* c_deviceDriverManufacturer;

enum class VRCommunicationProtocol {
  SERIAL = 0,
  BTSERIAL = 1,
};

enum class VREncodingProtocol {
  LEGACY = 0,
  ALPHA = 1,
};

enum class VRDeviceDriver {
  LUCIDGLOVES = 0,
  EMULATED_KNUCKLES = 1,
};

struct VRSerialConfiguration_t {
  std::string port;
  int baudRate;

  VRSerialConfiguration_t(const std::string port, const int baudRate) : port(port), baudRate(baudRate) {}
};

struct VRBTSerialConfiguration_t {
  std::string name;

  VRBTSerialConfiguration_t(const std::string name) : name(name) {}
};

struct VRPoseConfiguration_t {
  vr::HmdVector3_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
  bool calibrationButtonEnabled;

  VRPoseConfiguration_t(const vr::HmdVector3_t offsetVector, const vr::HmdQuaternion_t angleOffsetQuaternion, const float poseTimeOffset,
                        const bool controllerOverrideEnabled, const int controllerIdOverride, const bool calibrationButtonEnabled)
      : offsetVector(offsetVector),
        angleOffsetQuaternion(angleOffsetQuaternion),
        poseTimeOffset(poseTimeOffset),
        controllerIdOverride(controllerIdOverride),
        controllerOverrideEnabled(controllerOverrideEnabled),
        calibrationButtonEnabled(calibrationButtonEnabled) {}
};

struct VRDeviceConfiguration_t {
  vr::ETrackedControllerRole role;
  bool enabled;
  bool feedbackEnabled;
  VRPoseConfiguration_t poseConfiguration;
  VREncodingProtocol encodingProtocol;
  VRCommunicationProtocol communicationProtocol;
  VRDeviceDriver deviceDriver;

  VRDeviceConfiguration_t(const vr::ETrackedControllerRole role, const bool enabled, const bool feedbackEnabled, const VRPoseConfiguration_t poseConfiguration,
                          const VREncodingProtocol encodingProtocol, const VRCommunicationProtocol communicationProtocol, const VRDeviceDriver deviceDriver)
      : role(role),
        enabled(enabled),
        feedbackEnabled(feedbackEnabled),
        poseConfiguration(poseConfiguration),
        encodingProtocol(encodingProtocol),
        communicationProtocol(communicationProtocol),
        deviceDriver(deviceDriver) {}
};
