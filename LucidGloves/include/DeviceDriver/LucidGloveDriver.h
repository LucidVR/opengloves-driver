#pragma once
#include <openvr_driver.h>
#include <windows.h>
#include <thread>
#include <functional>
#include "driverlog.h"
#include "bones.h"

#include "Communication/SerialCommunicationManager.h"
#include "Encode/LegacyEncodingManager.h"
#include "DeviceDriver/DeviceDriver.h"

#include "ControllerPose.h"
#include "DeviceConfiguration.h"

static const char* c_rightControllerSerialNumber = "lucidgloves-right";
static const char* c_leftControllerSerialNumber = "lucidgloves-left";
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
	COMP_HAPTIC = 8,
	COMP_TRG_THUMB = 9,
	COMP_TRG_INDEX = 10,
	COMP_TRG_MIDDLE = 11,
	COMP_TRG_RING = 12,
	COMP_TRG_PINKY = 13
};

/**
This class controls the behavior of the controller. This is where you
tell OpenVR what your controller has (buttons, joystick, trackpad, etc.).
This is also where you inform OpenVR when the state of your controller
changes (for example, a button is pressed).

For the methods, take a look at the comment blocks for the ITrackedDeviceServerDriver
class too. Those comment blocks have some good information.

**/
class LucidGloveDeviceDriver : public IDeviceDriver {
public:
	LucidGloveDeviceDriver(const VRDeviceConfiguration_t& configuration);

	vr::EVRInitError Activate(uint32_t unObjectId);
	void Deactivate();

	void EnterStandby();
	void* GetComponent(const char* pchComponentNameAndVersion);
	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);
	vr::DriverPose_t GetPose();
	void RunFrame();

	bool m_hasActivated;

	const char* m_serialNumber;
private:
	uint32_t m_driverId;
	
	void StartDevice();
	bool IsRightHand() const;

	vr::VRInputComponentHandle_t m_skeletalComponentHandle{};
	vr::VRInputComponentHandle_t m_inputComponentHandles[14]{};

	vr::VRBoneTransform_t m_handTransforms[NUM_BONES];

	uint32_t m_shadowControllerId = vr::k_unTrackedDeviceIndexInvalid;

	std::unique_ptr<ICommunicationManager> m_communicationManager;
	std::unique_ptr<IEncodingManager> m_encodingManager;
	std::unique_ptr<ControllerPose> m_controllerPose;

	VRDeviceConfiguration_t m_configuration;
	
};
