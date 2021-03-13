#pragma once
#include <openvr_driver.h>
#include <windows.h>
#include <thread>
#include <functional>
#include "driverlog.h"
#include "bones.h"
#include "Comm/SerialCommunicationManager.h"
#include "ControllerPose.h"
#include "DeviceConfiguration.h"

static const char* c_rightControllerSerialNumber = "lucidgloves-right";
static const char* c_leftControllerSerialNumber = "lucidgloves-left";
static const char* c_settingsSection = "driver_lucidgloves";
static const char* c_deviceManufacturer = "Lucas_VRTech&Danwillm";
static const char* c_deviceControllerType = "lucidgloves";
static const char* c_deviceModelNumber = "lucidgloves1";
static const char* c_basePosePath = "/pose/raw";
static const char* c_inputProfilePath = "{lucidgloves}/input/lucidgloves_profile.json";
static const char* c_renderModelPath = "{lucidgloves}/rendermodels/lucidgloves";

static const enum ComponentIndex : int {
	COMP_JOY_X = 0,
	COMP_JOY_Y = 1,
	COMP_JOY_BTN = 2,
	COMP_BTN_TRG = 3,
	COMP_BTN_A = 4,
	COMP_BTN_B = 5,
	COMP_GES_GRAB = 6,
	COMP_GES_PINCH = 7,
	COMP_HAPTIC = 8
};

/**
This class controls the behavior of the controller. This is where you 
tell OpenVR what your controller has (buttons, joystick, trackpad, etc.).
This is also where you inform OpenVR when the state of your controller 
changes (for example, a button is pressed).

For the methods, take a look at the comment blocks for the ITrackedDeviceServerDriver 
class too. Those comment blocks have some good information.

**/
class ControllerDriver : public vr::ITrackedDeviceServerDriver
{
public:
	ControllerDriver(const VRDeviceConfiguration_t settings);
	/**
	Initialize your controller here. Give OpenVR information 
	about your controller and set up handles to inform OpenVR when 
	the controller state changes.
	**/
	vr::EVRInitError Activate(uint32_t unObjectId);

	/**
	Un-initialize your controller here.
	**/
	void Deactivate();

	/**
	Tell your hardware to go into stand-by mode (low-power).
	**/
	void EnterStandby();

	/**
	Take a look at the comment block for this method on ITrackedDeviceServerDriver. So as far 
	as I understand, driver classes like this one can implement lots of functionality that 
	can be categorized into components. This class just acts as an input device, so it will 
	return the IVRDriverInput class, but it could return other component classes if it had 
	more functionality, such as maybe overlays or UI functionality.
	**/
	void* GetComponent(const char* pchComponentNameAndVersion);

	/**
	Refer to ITrackedDeviceServerDriver. I think it sums up what this does well.
	**/
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);

	/**
	Returns the Pose for your device. Pose is an object that contains the position, rotation, velocity, 
	and angular velocity of your device.
	**/
	vr::DriverPose_t GetPose();

	/**
	You can retrieve the state of your device here and update OpenVR if anything has changed. This 
	method should be called every frame.
	**/
	void RunFrame();

	void StartDevice();

	//bool IsRightHand();

private:
	uint32_t m_driverId;

	vr::VRInputComponentHandle_t m_skeletalComponentHandle{};
	vr::VRInputComponentHandle_t m_inputComponentHandles[8]{};

	vr::VRBoneTransform_t m_handTransforms[NUM_BONES];

	uint32_t m_shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

	std::unique_ptr<ICommunicationManager> m_communicationManager;
	std::unique_ptr<ControllerPose> m_controllerPose;

	VRDeviceConfiguration_t m_configuration;
	short int DiscoverController() const;
	bool IsRightHand() const;
};
