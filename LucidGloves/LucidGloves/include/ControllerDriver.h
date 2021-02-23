#pragma once
#include <openvr_driver.h>
#include <windows.h>
#include <thread>
#include <Comm/SerialCommunicationManager.h>

static const char* right_controller_serial = "lucidgloves-right";
static const char* left_controller_serial = "lucidgloves-left";
static const char* c_settings_section = "driver_lucidgloves";
static const char* device_manufacturer = "Lucas_VRTech&Danwillm";
static const char* device_controller_type = "lucidgloves";
static const char* device_model_number = "lucidgloves1";

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
	ControllerDriver(vr::ETrackedControllerRole role);
	/**
	Initialize your controller here. Give OpenVR information 
	about your controller and set up handles to inform OpenVR when 
	the controller state changes.
	**/
	vr::EVRInitError Activate(uint32_t unObjectId);

	/**
	Tell the driver which hand glove it is, and run any other starting actions
	**/
	void Init(vr::ETrackedControllerRole role);

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


	/**
		Callback passed to communication manager. Called every time data is received from the device.
	**/
	void onDataReceived(int* data);


private:

	uint32_t m_driverId;
	vr::VRInputComponentHandle_t m_joystickYHandle;
	vr::VRInputComponentHandle_t m_joystickXHandle;
	vr::ETrackedControllerRole m_role = vr::TrackedControllerRole_OptOut; //changed in Init();
	std::unique_ptr<ICommunicationManager> communicationManager;
	std::thread m_serialThread;
	std::thread m_poseThread;
};