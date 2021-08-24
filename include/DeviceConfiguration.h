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

  VRSerialConfiguration_t(std::string port, int baudRate) : port(port), baudRate(baudRate){};
};

struct VRBTSerialConfiguration_t {
  std::string name;

  VRBTSerialConfiguration_t(std::string name) : name(name){};
};

struct VRPoseConfiguration_t {
  VRPoseConfiguration_t(vr::HmdVector3_t offsetVector, vr::HmdQuaternion_t angleOffsetQuaternion, float poseTimeOffset, bool controllerOverrideEnabled,
                        int controllerIdOverride)
      : offsetVector(offsetVector),
        angleOffsetQuaternion(angleOffsetQuaternion),
        poseTimeOffset(poseTimeOffset),
        controllerOverrideEnabled(controllerOverrideEnabled),
        controllerIdOverride(controllerIdOverride){};
  vr::HmdVector3_t offsetVector;
  vr::HmdQuaternion_t angleOffsetQuaternion;
  float poseTimeOffset;
  int controllerIdOverride;
  bool controllerOverrideEnabled;
};

struct VRDeviceConfiguration_t {
  VRDeviceConfiguration_t(vr::ETrackedControllerRole role, bool enabled, VRPoseConfiguration_t poseConfiguration, VREncodingProtocol encodingProtocol,
                          VRCommunicationProtocol communicationProtocol, VRDeviceDriver deviceDriver)
      : role(role),
        enabled(enabled),
        poseConfiguration(poseConfiguration),
        encodingProtocol(encodingProtocol),
        communicationProtocol(communicationProtocol),
        deviceDriver(deviceDriver){};

  vr::ETrackedControllerRole role;
  bool enabled;

  VRPoseConfiguration_t poseConfiguration;

  VREncodingProtocol encodingProtocol;
  VRCommunicationProtocol communicationProtocol;
  VRDeviceDriver deviceDriver;
};
