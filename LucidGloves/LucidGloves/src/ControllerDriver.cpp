#include <ControllerDriver.h>

ControllerDriver::ControllerDriver(vr::ETrackedControllerRole role) : m_role(role) {};

vr::EVRInitError ControllerDriver::Activate(uint32_t unObjectId)
{
	m_driverId = unObjectId; //unique ID for your driver
	const char* serial_number = m_role == vr::TrackedControllerRole_RightHand ? right_controller_serial : left_controller_serial;
	vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId); //this gets a container object where you store all the information about your driver

	vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, "{lucidgloves}/input/controller_profile.json"); //tell OpenVR where to get your driver's Input Profile
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, m_role); //tells OpenVR what kind of device this is
	vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, serial_number);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, device_model_number);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, device_manufacturer);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)100000);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, device_controller_type);

	return vr::VRInitError_None;
}

vr::DriverPose_t ControllerDriver::GetPose()
{
	//placeholder
	vr::DriverPose_t pose = { 0 };
	pose.poseIsValid = false;
	pose.result = vr::TrackingResult_Calibrating_OutOfRange;
	pose.deviceIsConnected = true;
	return pose;
}

void ControllerDriver::RunFrame()
{
	//Since we used VRScalarUnits_NormalizedTwoSided as the unit, the range is -1 to 1.
	vr::VRDriverInput()->UpdateScalarComponent(m_joystickYHandle, 0.95f, 0); //placeholder
	vr::VRDriverInput()->UpdateScalarComponent(m_joystickXHandle, 0.0f, 0); //placeholder
}

void ControllerDriver::Deactivate()
{
	m_driverId = vr::k_unTrackedDeviceIndexInvalid;
}

void* ControllerDriver::GetComponent(const char* pchComponentNameAndVersion)
{
	//I found that if this method just returns null always, it works fine. But I'm leaving the if statement in since it doesn't hurt.
	//Check out the IVRDriverInput_Version declaration in openvr_driver.h. You can search that file for other _Version declarations 
	//to see other components that are available. You could also put a log in this class and output the value passed into this 
	//method to see what OpenVR is looking for.
	if (strcmp(vr::IVRDriverInput_Version, pchComponentNameAndVersion) == 0)
	{
		return this;
	}
	return NULL;
}

void ControllerDriver::EnterStandby() {}

void ControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) 
{
	if (unResponseBufferSize >= 1)
	{
		pchResponseBuffer[0] = 0;
	}
}