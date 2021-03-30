#pragma once
#include "openvr_driver.h"
#include "Comm/SerialCommunicationManager.h"

enum VRCommunicationProtocol {
	SERIAL = 0,
};

enum VREncodingProtocol {
	LEGACY = 0,
};

struct VRDeviceConfiguration_t {
	VRDeviceConfiguration_t(vr::ETrackedControllerRole role, bool enabled, vr::HmdVector3_t offsetVector, vr::HmdVector3_t angleOffsetVector, float poseOffset, int adcCounts, VRCommunicationProtocol communicationProtocol, VREncodingProtocol encodingProtocol, VRSerialConfiguration_t serialConfiguration) :
		role(role),
		enabled(enabled),
		offsetVector(offsetVector),
		angleOffsetVector(angleOffsetVector),
		poseOffset(poseOffset),
		adcCounts(adcCounts),
		serialConfiguration(serialConfiguration),
		communicationProtocol(communicationProtocol),
		encodingProtocol(encodingProtocol) {};

	vr::ETrackedControllerRole role;
	bool enabled;
	vr::HmdVector3_t offsetVector;
	vr::HmdVector3_t angleOffsetVector;
	int adcCounts;
	float poseOffset;

	VRSerialConfiguration_t serialConfiguration;

	VRCommunicationProtocol communicationProtocol;
	VREncodingProtocol encodingProtocol;

};
