#pragma once
#include <openvr_driver.h>
#include "Quaternion.h"

class ControllerPose {
public:
	ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer);
	vr::DriverPose_t UpdatePose();
private:
	//We may not initially know what the id of the device that we want to shadow is. This method finds devices that have a specific type specified and that are not this one
	short int DiscoverController(vr::ETrackedControllerRole shadowDeviceOfRole, std::string thisDeviceManufacturer) const;

	//We don't have our own tracking system, so we fetch positioning from the propriety controllers/trackers.
	//This member variable is the id of the controller we are "shadowing" and receiving positioning from.
	short int m_shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

	//This is the pose we send back for OpenVR to process as our pose for the driver.
	//This represents the orientation and rotation, as well as angular velocity, etc.
	vr::DriverPose_t m_pose;

	//This vector is set to the offset of the controller relative to the hand.
	//If we have the controller mounted on top of the hand with no offset, the hand will appear where the controller is - not what we want.
	//This offset vector is configurable in settings.
	vr::HmdVector3_t m_offsetVector = { 0, 0, 0 };
};