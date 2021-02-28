#pragma once
#include <openvr_driver.h>
#include "Quaternion.h"
#include "DeviceConfiguration.h"
#include "DriverLog.h"

class ControllerPose {
public:
	ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer, VRDeviceConfiguration_t configuration, uint32_t driverId);
	void UpdatePose();
private:
	short int m_driverId = -1;
	//We may not initially know what the id of the device that we want to shadow is. This method finds devices that have a specific type specified and that are not this one
	void DiscoverController();

	//We don't have our own tracking system, so we fetch positioning from the propriety controllers/trackers.
	//This member variable is the id of the controller we are "shadowing" and receiving positioning from.
	short int m_shadowControllerId = -1;

	//This is the pose we send back for OpenVR to process as our pose for the driver.
	//This represents the orientation and rotation, as well as angular velocity, etc.
	vr::DriverPose_t m_pose;

	VRDeviceConfiguration_t m_configuration;

	vr::ETrackedControllerRole m_shadowDeviceOfRole = vr::TrackedControllerRole_Invalid;

	std::string m_thisDeviceManufacturer = "";

};