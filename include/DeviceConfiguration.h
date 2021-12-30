#pragma once

#include <utility>

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
  Serial,
  BtSerial,
  NamedPipe,
};

enum class VREncodingProtocol {
  Legacy = 0,
  Alpha = 1,
};

enum class VRDeviceDriver {
  LucidGloves = 0,
  EmulatedKnuckles = 1,
};

struct VRSerialConfiguration {
  std::string port;
  int baudRate;

  VRSerialConfiguration(std::string port, const int baudRate) : port(std::move(port)), baudRate(baudRate) {}
};

struct VRBTSerialConfiguration {
  std::string name;

  explicit VRBTSerialConfiguration(std::string name) : name(std::move(name)) {}
};

struct VRNamedPipeInputConfiguration {
  std::string pipeName;

  VRNamedPipeInputConfiguration(std::string pipeName) : pipeName(std::move(pipeName)) {}
};

struct VRPoseConfiguration {
  vr::HmdVector3_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
  bool calibrationButtonEnabled;

  VRPoseConfiguration(
      const vr::HmdVector3_t offsetVector,
      const vr::HmdQuaternion_t angleOffsetQuaternion,
      const float poseTimeOffset,
      const bool controllerOverrideEnabled,
      const int controllerIdOverride,
      const bool calibrationButtonEnabled)
      : offsetVector(offsetVector),
        angleOffsetQuaternion(angleOffsetQuaternion),
        poseTimeOffset(poseTimeOffset),
        controllerIdOverride(controllerIdOverride),
        controllerOverrideEnabled(controllerOverrideEnabled),
        calibrationButtonEnabled(calibrationButtonEnabled) {}
};

struct VRDeviceConfiguration {
  vr::ETrackedControllerRole role;
  bool enabled;
  bool feedbackEnabled;
  VRPoseConfiguration poseConfiguration;
  VREncodingProtocol encodingProtocol;
  VRCommunicationProtocol communicationProtocol;
  VRDeviceDriver deviceDriver;

  VRDeviceConfiguration(
      const vr::ETrackedControllerRole role,
      const bool enabled,
      const bool feedbackEnabled,
      const VRPoseConfiguration poseConfiguration,
      const VREncodingProtocol encodingProtocol,
      const VRCommunicationProtocol communicationProtocol,
      const VRDeviceDriver deviceDriver)
      : role(role),
        enabled(enabled),
        feedbackEnabled(feedbackEnabled),
        poseConfiguration(poseConfiguration),
        encodingProtocol(encodingProtocol),
        communicationProtocol(communicationProtocol),
        deviceDriver(deviceDriver) {}
};
