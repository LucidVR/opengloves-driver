#pragma once

#include "Communication/CommunicationManager.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"
#include "openvr_driver.h"

static const char *c_driverSettingsSection = "driver_openglove";
static const char *c_poseSettingsSection = "pose_settings";

enum VRCommunicationProtocol {
    SERIAL = 0,
    EXPERIMENTAL_SERIAL = 2
};

enum VREncodingProtocol {
    LEGACY = 0,
};

enum VRDeviceDriver {
    LUCIDGLOVES = 0,
    EMULATED_KNUCKLES = 1,
};

struct VRSerialConfiguration_t {
    std::string port;

    VRSerialConfiguration_t(std::string port) : port(port) {};
};

struct VRPoseConfiguration_t {
    VRPoseConfiguration_t(vr::HmdVector3_t offsetVector, vr::HmdQuaternion_t angleOffsetQuaternion, float poseOffset,
                          bool controllerOverrideEnabled, int controllerIdOverride) :
            offsetVector(offsetVector),
            angleOffsetQuaternion(angleOffsetQuaternion),
            poseOffset(poseOffset),
            controllerOverrideEnabled(controllerOverrideEnabled),
            controllerIdOverride(controllerIdOverride) {};
    vr::HmdVector3_t offsetVector;
    vr::HmdQuaternion_t angleOffsetQuaternion;
    float poseOffset;
    int controllerIdOverride;
    bool controllerOverrideEnabled;
};

struct VRDeviceConfiguration_t {
    VRDeviceConfiguration_t(vr::ETrackedControllerRole role,
                            bool enabled,
                            VRPoseConfiguration_t poseConfiguration,
                            VREncodingProtocol encodingProtocol,
                            VRCommunicationProtocol communicationProtocol,
                            VRDeviceDriver deviceDriver) :
            role(role),
            enabled(enabled),
            poseConfiguration(poseConfiguration),
            encodingProtocol(encodingProtocol),
            communicationProtocol(communicationProtocol),
            deviceDriver(deviceDriver) {};

    vr::ETrackedControllerRole role;
    bool enabled;

    VRPoseConfiguration_t poseConfiguration;

    VREncodingProtocol encodingProtocol;
    VRCommunicationProtocol communicationProtocol;
    VRDeviceDriver deviceDriver;
};
