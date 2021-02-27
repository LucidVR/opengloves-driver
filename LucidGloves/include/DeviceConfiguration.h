#pragma once
#include "openvr_driver.h"
#include "Comm/SerialCommunicationManager.h"

enum VRDeviceProtocol {
	SERIAL = 0,
};

struct VRDeviceConfiguration_t {
	VRDeviceConfiguration_t(vr::ETrackedControllerRole role, vr::HmdVector3_t offsetVector, float poseOffset, VRSerialConfiguration_t serialConfiguration) :
		role(role),
		offsetVector(offsetVector),
		poseOffset(poseOffset),
		serialConfiguration(serialConfiguration),
		protocol(VRDeviceProtocol::SERIAL) {};

	vr::ETrackedControllerRole role;

	vr::HmdVector3_t offsetVector;

	float poseOffset;

	VRSerialConfiguration_t serialConfiguration;

	VRDeviceProtocol protocol;

};
