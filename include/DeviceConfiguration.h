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
  SERIAL,
  BT_SERIAL,
  NAMED_PIPE
};

enum class VREncodingProtocol {
  LEGACY = 0,
  ALPHA = 1,
};

enum class VRDeviceDriver {
  LUCIDGLOVES = 0,
  EMULATED_KNUCKLES = 1,
};

struct VRSerialConfiguration {
  std::string port;
  int baudRate;

  VRSerialConfiguration(std::string port, int baudRate) : port(port), baudRate(baudRate){};
};

struct VRBTSerialConfiguration {
  std::string name;

  VRBTSerialConfiguration(std::string name) : name(name){};
};

struct VRNamedPipeInputConfiguration {
  std::string pipeName;

  VRNamedPipeInputConfiguration(std::string pipeName) : pipeName(pipeName){};

};

struct VRPoseConfiguration {
  VRPoseConfiguration(vr::HmdVector3_t offsetVector, vr::HmdQuaternion_t angleOffsetQuaternion, float poseTimeOffset, bool controllerOverrideEnabled,
                        int controllerIdOverride, bool calibrationButtonEnabled)
      : offsetVector(offsetVector),
        angleOffsetQuaternion(angleOffsetQuaternion),
        poseTimeOffset(poseTimeOffset),
        controllerOverrideEnabled(controllerOverrideEnabled),
        controllerIdOverride(controllerIdOverride),
        calibrationButtonEnabled(calibrationButtonEnabled){};
  vr::HmdVector3_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
  bool calibrationButtonEnabled;
};

struct VRDeviceConfiguration {
  VRDeviceConfiguration(vr::ETrackedControllerRole role, bool enabled, VRPoseConfiguration poseConfiguration, VREncodingProtocol encodingProtocol,
                          VRCommunicationProtocol communicationProtocol, VRDeviceDriver deviceDriver)
      : role(role),
        enabled(enabled),
        poseConfiguration(poseConfiguration),
        encodingProtocol(encodingProtocol),
        communicationProtocol(communicationProtocol),
        deviceDriver(deviceDriver){};

  vr::ETrackedControllerRole role;
  bool enabled;

  VRPoseConfiguration poseConfiguration;

  VREncodingProtocol encodingProtocol;
  VRCommunicationProtocol communicationProtocol;
  VRDeviceDriver deviceDriver;
};
