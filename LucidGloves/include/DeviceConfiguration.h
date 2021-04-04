#pragma once
#include "Communication/CommunicationManager.h"
#include "DeviceDriver/DeviceDriver.h"
#include "Encode/EncodingManager.h"
#include "openvr_driver.h"
#include <memory>

static const char* c_driverSettingsSection = "driver_opengloves";
static const char* c_poseSettingsSection = "pose_settings";

enum VRCommunicationProtocol {
	SERIAL = 0,
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


struct VRDeviceConfiguration_t {
	VRDeviceConfiguration_t(vr::ETrackedControllerRole role,
							bool enabled,
							vr::HmdVector3_t offsetVector,
							vr::HmdVector3_t angleOffsetVector,
							float poseOffset,
							VREncodingProtocol encodingProtocol,
							VRCommunicationProtocol communicationProtocol,
							VRDeviceDriver deviceDriver) :
		role(role),
		enabled(enabled),
		offsetVector(offsetVector),
		angleOffsetVector(angleOffsetVector),
		poseOffset(poseOffset),
		communicationProtocol(communicationProtocol),
		deviceDriver(deviceDriver) {};

	vr::ETrackedControllerRole role;
	bool enabled;
	vr::HmdVector3_t offsetVector;
	vr::HmdVector3_t angleOffsetVector;
	float poseOffset;
	
	VREncodingProtocol encodingProtocol;
	VRCommunicationProtocol communicationProtocol;
	VRDeviceDriver deviceDriver;
};
