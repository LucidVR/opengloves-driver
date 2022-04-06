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

  union {
    VRCommunicationSerialConfiguration serial;
    VRCommunicationBTSerialConfiguration btSerial;
    VRCommunicationNamedPipeConfiguration namedPipe;
  };
};

struct VRDeviceKnucklesConfiguration {
  bool indexCurlTrigger;
  bool approximateThumb;

  std::string serialNumber;
};

struct VRDeviceLucidglovesConfiguration {
  std::string serialNumber;
};

struct VRDeviceConfiguration {
  VRDeviceType deviceType;

  union configuration {
    configuration(const VRDeviceKnucklesConfiguration& knuckles) : knuckles(knuckles){};
    configuration(const VRDeviceLucidglovesConfiguration& lucidgloves) : lucidgloves(lucidgloves){};

    VRDeviceKnucklesConfiguration knuckles;
    VRDeviceLucidglovesConfiguration lucidgloves;
  };
};

struct VRAlphaEncodingConfiguration {
  unsigned int maxAnalogValue;
};

struct VRLegacyEncodingConfiguration {
  unsigned int maxAnalogValue;
};

struct VREncodingConfiguration {
  VREncodingProtocol encodingProtocol;
  union {
    VRAlphaEncodingConfiguration alpha;
    VRLegacyEncodingConfiguration legacy;
  };
};

struct VRPoseConfiguration {
  vr::HmdVector3d_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
  bool calibrationButtonEnabled;
};

struct VRDriverConfiguration {
  bool enabled;
  bool feedbackEnabled;
  vr::ETrackedControllerRole role;

  VRDeviceConfiguration deviceConfiguration;
  VREncodingConfiguration encodingConfiguration;
  VRCommunicationConfiguration communicationConfiguration;
  VRPoseConfiguration poseConfiguration;
};

VRDriverConfiguration GetDeviceConfiguration(const vr::ETrackedControllerRole& role);