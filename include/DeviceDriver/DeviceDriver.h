#pragma once
#include "openvr_driver.h"
#include "DeviceConfiguration.h"

class IDeviceDriver : public vr::ITrackedDeviceServerDriver {
public:
	/**
	Initialize your controller here. Give OpenVR information
	about your controller and set up handles to inform OpenVR when
	the controller state changes.
	**/
	virtual vr::EVRInitError Activate(uint32_t unObjectId) = 0;

	/**
	Un-initialize your controller here.
	**/
	virtual void Deactivate() = 0;

	/**
	Tell your hardware to go into stand-by mode (low-power).
	**/
	virtual void EnterStandby() = 0;

	/**
	Take a look at the comment block for this method on ITrackedDeviceServerDriver. So as far
	as I understand, driver classes like this one can implement lots of functionality that
	can be categorized into components. This class just acts as an input device, so it will
	return the IVRDriverInput class, but it could return other component classes if it had
	more functionality, such as maybe overlays or UI functionality.
	**/
	virtual void* GetComponent(const char* pchComponentNameAndVersion) = 0;

	/**
	Refer to ITrackedDeviceServerDriver. I think it sums up what this does well.
	**/
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) = 0;

	/**
	Returns the Pose for your device. Pose is an object that contains the position, rotation, velocity,
	and angular velocity of your device.
	**/
	virtual vr::DriverPose_t GetPose() = 0;

	/**
	You can retrieve the state of your device here and update OpenVR if anything has changed. This
	method should be called every frame.
	**/
	virtual void RunFrame() = 0;

	virtual std::string GetSerialNumber() = 0;
	virtual bool IsActive() = 0;
};