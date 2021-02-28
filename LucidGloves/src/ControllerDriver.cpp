#include "ControllerDriver.h"

ControllerDriver::ControllerDriver(const VRDeviceConfiguration_t configuration)
	: m_configuration(configuration) {

	//copy a default bone transform to our hand transform for use in finger positioning later
	std::copy(
		std::begin(m_configuration.role == vr::TrackedControllerRole_RightHand ? right_open_hand_pose : left_open_hand_pose),
		std::end(m_configuration.role == vr::TrackedControllerRole_RightHand ? right_open_hand_pose : left_open_hand_pose),
		std::begin(m_handTransforms)
	);


	switch (m_configuration.protocol) {
	case VRDeviceProtocol::SERIAL:
		m_communicationManager = std::make_unique<SerialManager>(m_configuration.serialConfiguration);

		break;
	}
}

bool ControllerDriver::IsRightHand() const {
	return m_configuration.role == vr::TrackedControllerRole_RightHand;
}

vr::EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{
	DebugDriverLog("Activating lucidgloves...");
	const bool isRightHand = IsRightHand();

	m_driverId = unObjectId; //unique ID for your driver
	m_controllerPose = std::make_unique<ControllerPose>(m_configuration.role, std::string(c_deviceManufacturer), m_configuration, m_driverId);

	vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId); //this gets a container object where you store all the information about your driver

	vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, c_inputProfilePath); //tell OpenVR where to get your driver's Input Profile
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, isRightHand ? vr::TrackedControllerRole_RightHand : vr::TrackedControllerRole_LeftHand); //tells OpenVR what kind of device this is
	vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, isRightHand ? c_rightControllerSerialNumber : c_leftControllerSerialNumber);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, c_deviceModelNumber);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceManufacturer);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)100000);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, c_deviceControllerType);


	vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &m_inputComponentHandles[ComponentIndex::COMP_JOY_X], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &m_inputComponentHandles[ComponentIndex::COMP_JOY_Y], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/joystick/click", &m_inputComponentHandles[ComponentIndex::COMP_JOY_BTN]);
	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/A/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_A]);

	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grab/click", &m_inputComponentHandles[ComponentIndex::COMP_GES_GRAB]);

	vr::VRDriverInput()->CreateHapticComponent(props, "output/haptic", &m_inputComponentHandles[ComponentIndex::COMP_HAPTIC]);

	// Create the skeletal component and save the handle for later use

	vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(props,
		isRightHand ? "/input/skeleton/right" : "/input/skeleton/left",
		isRightHand ? "/skeleton/hand/right" : "/skeleton/hand/left",
		"/pose/raw",
		vr::VRSkeletalTracking_Partial,
		isRightHand ? right_fist_pose : left_fist_pose,
		NUM_BONES,
		&m_skeletalComponentHandle);

	if (error != vr::VRInputError_None)
	{
		// Handle failure case
		DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
	}

	StartDevice();

	return vr::VRInitError_None;
}

//This could do with a rename, its a bit vague as to what it does
void ControllerDriver::StartDevice() {
	m_communicationManager->Connect();

	if (m_communicationManager->IsConnected()) {

		m_communicationManager->BeginListener([&](VRCommData_t datas) {
			DebugDriverLog("Received data!");
			ComputeEntireHand(m_handTransforms, datas.flexion, datas.splay, IsRightHand());

			vr::EVRInputError err;

			err = vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
			if (err != vr::VRInputError_None) DebugDriverLog("UpdateSkeletonComponent failed.  Error: %s\n", err);
			err = vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, m_handTransforms, NUM_BONES);
			if (err != vr::VRInputError_None) DebugDriverLog("UpdateSkeletonComponent failed.  Error: %s\n", err);

			vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_X], datas.joyX, 0);
			vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_Y], datas.joyY, 0);

			vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_A], datas.aButton, 0);
			vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_B], datas.bButton, 0);

			vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_GES_GRAB], datas.grab, 0);
			vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_GES_PINCH], datas.pinch, 0);

		});

	}
	else {
		DebugDriverLog("Device did not connect successfully");
		//Perhaps retry
	}
}

vr::DriverPose_t ControllerDriver::GetPose()
{
	vr::DriverPose_t pose = { 0 };

	return pose;
}

void ControllerDriver::RunFrame()
{
	m_controllerPose->UpdatePose();
}


void ControllerDriver::Deactivate()
{
	m_communicationManager->Disconnect();
	m_driverId = vr::k_unTrackedDeviceIndexInvalid;
}

void* ControllerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
	//I found that if this method just returns null always, it works fine. But I'm leaving the if statement in since it doesn't hurt.
	//Check out the IVRDriverInput_Version declaration in openvr_driver.h. You can search that file for other _Version declarations 
	//to see other components that are available. You could also put a log in this class and output the value passed into this 
	//method to see what OpenVR is looking for.

	/*if (strcmp(vr::IVRDriverInput_Version, pchComponentNameAndVersion) == 0)
	{
		return this;
	}
	return NULL;*/

	return nullptr;
}

void ControllerDriver::EnterStandby() {}

void ControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
	if (unResponseBufferSize >= 1)
	{
		pchResponseBuffer[0] = 0;
	}
}