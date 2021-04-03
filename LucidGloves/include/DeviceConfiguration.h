#pragma once
#include "openvr_driver.h"
#include "Encode/EncodingManager.h"
#include "Communication/CommunicationManager.h"
#include "DeviceDriver/DeviceDriver.h"

static const char* c_settingsSection = "driver_opengloves";

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
							ICommunicationManager* communicationManager,
							IEncodingManager* encodingManager,
							VRDeviceDriver selectedDeviceDriver) :
		role(role),
		enabled(enabled),
		offsetVector(offsetVector),
		angleOffsetVector(angleOffsetVector),
		poseOffset(poseOffset),
		communicationProtocol(communicationProtocol),
		encodingProtocol(encodingProtocol) {};

	vr::ETrackedControllerRole role;
	bool enabled;
	vr::HmdVector3_t offsetVector;
	vr::HmdVector3_t angleOffsetVector;
	float poseOffset;

	ICommunicationManager* communicationProtocol;
	IEncodingManager* encodingProtocol;
	VRDeviceDriver selectedDeviceDriver;
};
