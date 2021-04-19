#pragma once
#include <openvr_driver.h>
#include "DeviceConfiguration.h"

class ControllerPose {
public:
	ControllerPose(vr::ETrackedControllerRole shadowDeviceOfRole,
							   std::string thisDeviceManufacturer,
							   vr::HmdVector3_t offsetVector,
							   vr::HmdVector3_t angleOffsetVector, 
							   int controllerIdOverride,
							   bool isControllerOverride,
							   uint32_t driverId);
	vr::DriverPose_t UpdatePose();
private:
	short int m_driverId = -1;
	//We may not initially know what the id of the device that we want to shadow is. This method finds devices that have a specific type specified and that are not this one
	void DiscoverController();

	//We don't have our own tracking system, so we fetch positioning from the propriety controllers/trackers.
	//This member variable is the id of the controller we are "shadowing" and receiving positioning from.
	short int m_shadowControllerId = -1;

	int m_controllerIdOverride;
	bool m_isControllerOverride;
	vr::HmdQuaternion_t m_offsetQuaternion;

	vr::ETrackedControllerRole m_shadowDeviceOfRole = vr::TrackedControllerRole_Invalid;

	std::string m_thisDeviceManufacturer = "";

	vr::HmdVector3_t m_offsetVector;

};