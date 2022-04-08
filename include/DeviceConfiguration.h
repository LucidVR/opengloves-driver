#pragma once

#include <variant>

// quaternion used for == overloads
#include "Util/Quaternion.h"
#include "openvr_driver.h"

extern const char* c_poseSettingsSection;
extern const char* c_driverSettingsSection;
extern const char* c_serialCommunicationSettingsSection;
extern const char* c_btserialCommunicationSettingsSection;
extern const char* c_knuckleDeviceSettingsSection;
extern const char* c_lucidGloveDeviceSettingsSection;
extern const char* c_alphaEncodingSettingsSection;
extern const char* c_legacyEncodingSettingsSection;

extern const char* c_deviceManufacturer;

enum class VRCommunicationProtocol {
  Serial,
  BtSerial,
  NamedPipe,
};

enum class VREncodingProtocol {
  Legacy = 0,
  Alpha = 1,
};

enum class VRDeviceType {
  LucidGloves = 0,
  EmulatedKnuckles = 1,
};

struct VRCommunicationSerialConfiguration {
  std::string port;
  int baudRate;

  bool operator==(const VRCommunicationSerialConfiguration&) const = default;
};

struct VRCommunicationBTSerialConfiguration {
  std::string name;

  bool operator==(const VRCommunicationBTSerialConfiguration&) const = default;
};

struct VRCommunicationNamedPipeConfiguration {
  std::string pipeName;

  bool operator==(const VRCommunicationNamedPipeConfiguration&) const = default;
};

struct VRCommunicationConfiguration {
  VRCommunicationProtocol communicationProtocol;

  bool feedbackEnabled;

  std::variant<VRCommunicationSerialConfiguration, VRCommunicationBTSerialConfiguration, VRCommunicationNamedPipeConfiguration> configuration;

  bool operator==(const VRCommunicationConfiguration&) const = default;
};

struct VRAlphaEncodingConfiguration {
  bool operator==(const VRAlphaEncodingConfiguration&) const = default;
};

struct VRLegacyEncodingConfiguration {
  bool operator==(const VRLegacyEncodingConfiguration&) const = default;
};

struct VREncodingConfiguration {
  VREncodingProtocol encodingProtocol;
  unsigned int maxAnalogValue;

  std::variant<VRAlphaEncodingConfiguration, VRLegacyEncodingConfiguration> configuration;

  bool operator==(const VREncodingConfiguration&) const = default;
};

struct VRPoseConfiguration {
  vr::HmdVector3d_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
  bool calibrationButtonEnabled;

  bool operator==(const VRPoseConfiguration&) const = default;
};

struct VRDeviceKnucklesConfiguration {
  bool indexCurlTrigger;
  bool approximateThumb;

  bool operator==(const VRDeviceKnucklesConfiguration&) const = default;
};

struct VRDeviceLucidglovesConfiguration {
  std::string serialNumber;

  bool operator==(const VRDeviceLucidglovesConfiguration&) const = default;
};

struct VRDeviceConfiguration {
  VRDeviceType deviceType;

  std::string serialNumber;
  vr::ETrackedControllerRole role;

  VRPoseConfiguration poseConfiguration;

  std::variant<VRDeviceKnucklesConfiguration, VRDeviceLucidglovesConfiguration> configuration;

  bool operator==(const VRDeviceConfiguration&) const = default;
};

struct VRDriverConfiguration {
  bool enabled;

  VREncodingConfiguration encodingConfiguration;
  VRCommunicationConfiguration communicationConfiguration;
  VRDeviceConfiguration deviceConfiguration;

  bool operator==(const VRDriverConfiguration&) const = default;
};

VRDriverConfiguration GetDriverConfiguration(const vr::ETrackedControllerRole& role);