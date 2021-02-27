#include "ControllerDriver.h"

ControllerDriver::ControllerDriver(const VRDeviceConfiguration_t configuration)
	: m_configuration(configuration) {

	//copy a default bone transform to our hand transform for use in finger positioning later
	std::copy(
		std::begin(m_configuration.role == vr::TrackedControllerRole_RightHand ? right_open_hand_pose : left_open_hand_pose),
		std::end(m_configuration.role == vr::TrackedControllerRole_RightHand ? right_open_hand_pose : left_open_hand_pose),
		std::begin(m_handTransforms)
	);

	m_controllerPose = std::make_unique<ControllerPose>(m_configuration.role, std::string(c_deviceManufacturer));

	switch (m_configuration.protocol) {
	case VRDeviceProtocol::SERIAL:
		m_communicationManager = std::make_unique<SerialManager>(m_configuration.serialConfiguration);

		break;
	}
}

bool ControllerDriver::IsRightHand() const {
	return m_configuration.role == vr::TrackedControllerRole_RightHand;
}

vr::EVRInitError ControllerDriver::Activate(const uint32_t unObjectId)
{
	m_driverId = unObjectId; //unique ID for your driver

	vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId); //this gets a container object where you store all the information about your driver

	vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, "{lucidgloves}/input/controller_profile.json"); //tell OpenVR where to get your driver's Input Profile
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, m_configuration.role); //tells OpenVR what kind of device this is
	vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, m_configuration.role == vr::TrackedControllerRole_RightHand ? c_rightControllerSerialNumber : c_leftControllerSerialNumber);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, c_deviceModelNumber);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceManufacturer);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)100000);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, c_deviceControllerType);

	// Create the skeletal component and save the handle for later use
	vr::EVRInputError err = vr::VRDriverInput()->CreateSkeletonComponent(props, c_componentName, c_skeletonPath, c_basePosePath,
		vr::EVRSkeletalTrackingLevel::VRSkeletalTracking_Partial, NULL, NUM_BONES, &m_skeletalComponentHandle);

	if (err != vr::VRInputError_None)
	{
		// Handle failure case TODO: switch to using driverlog.cpp
		DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", err);
	}

	StartDevice();

	return vr::VRInitError_None;
}

//This could do with a rename, its a bit vague as to what it does
void ControllerDriver::StartDevice() {
	m_communicationManager->Connect();

	if (m_communicationManager->IsConnected()) {

		m_communicationManager->BeginListener([&](VRCommData_t datas) {
			ComputeEntireHand(m_handTransforms, datas.flexion, datas.splay, IsRightHand());

			vr::EVRInputError err = vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
			if (err != vr::VRInputError_None)
			{
				DebugDriverLog("UpdateSkeletonComponent failed.  Error: %s\n", err);
			}
			});

	}
	else {
		DebugDriverLog("Device did not connect successfully");
		//Perhaps retry
	}
}

vr::DriverPose_t ControllerDriver::GetPose()
{
	return m_controllerPose->UpdatePose();
}

void ControllerDriver::RunFrame()
{
	//do nothing?
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