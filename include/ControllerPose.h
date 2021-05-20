#pragma once
#include <openvr_driver.h>
#include "DeviceConfiguration.h"

class ControllerPose {
public:
	ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole,
							   std::string thisDeviceManufacturer,
							   VRPoseConfiguration_t poseConfiguration);
	vr::DriverPose_t UpdatePose();
private:
	//We may not initially know what the id of the device that we want to shadow is. This method finds devices that have a specific type specified and that are not this one
	void DiscoverController();

	uint32_t m_shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

	VRPoseConfiguration_t m_poseConfiguration;

	vr::ETrackedControllerRole m_shadowDeviceOfRole = vr::TrackedControllerRole_Invalid;

	std::string m_thisDeviceManufacturer;

	bool ControllerPose::IsOtherRole(int32_t test);

};