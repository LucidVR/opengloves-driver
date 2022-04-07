#pragma once

#include <variant>

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
};

struct VRCommunicationBTSerialConfiguration {
  std::string name;
};

struct VRCommunicationNamedPipeConfiguration {
  std::string pipeName;
};

struct VRCommunicationConfiguration {
  VRCommunicationProtocol communicationProtocol;

  bool feedbackEnabled;

  std::variant<VRCommunicationSerialConfiguration, VRCommunicationBTSerialConfiguration, VRCommunicationNamedPipeConfiguration> configuration;
};

struct VRAlphaEncodingConfiguration {};

struct VRLegacyEncodingConfiguration {};

struct VREncodingConfiguration {
  VREncodingProtocol encodingProtocol;
  unsigned int maxAnalogValue;

  std::variant<VRAlphaEncodingConfiguration, VRLegacyEncodingConfiguration> configuration;
};

struct VRPoseConfiguration {
  vr::HmdVector3d_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
  bool calibrationButtonEnabled;
};

struct VRDeviceKnucklesConfiguration {
  bool indexCurlTrigger;
  bool approximateThumb;
};

struct VRDeviceLucidglovesConfiguration {
  std::string serialNumber;
};

struct VRDeviceConfiguration {
  VRDeviceType deviceType;

  std::string serialNumber;
  vr::ETrackedControllerRole role;

  VRPoseConfiguration poseConfiguration;

  std::variant<VRDeviceKnucklesConfiguration, VRDeviceLucidglovesConfiguration> configuration;
};

struct VRDriverConfiguration {
  bool enabled;

  VREncodingConfiguration encodingConfiguration;
  VRCommunicationConfiguration communicationConfiguration;
  VRDeviceConfiguration deviceConfiguration;
};

VRDriverConfiguration GetDriverConfiguration(const vr::ETrackedControllerRole& role);