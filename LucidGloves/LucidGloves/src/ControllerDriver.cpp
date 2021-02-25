#include <ControllerDriver.h>


ControllerDriver::ControllerDriver(const vr::ETrackedControllerRole role) : m_role(role) {};

void OnDataReceived(const float* datas) {

}

bool ControllerDriver::isRightHand() {
	return vr::TrackedControllerRole_RightHand ? c_rightControllerSerialNumber : c_leftControllerSerialNumber;
}

vr::EVRInitError ControllerDriver::Activate(const uint32_t unObjectId)
{
	m_driverId = unObjectId; //unique ID for your driver

	vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId); //this gets a container object where you store all the information about your driver

	vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, "{lucidgloves}/input/controller_profile.json"); //tell OpenVR where to get your driver's Input Profile
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, m_role); //tells OpenVR what kind of device this is
	vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, m_role == vr::TrackedControllerRole_RightHand ? c_rightControllerSerialNumber : c_leftControllerSerialNumber);
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
		vr::VRDriverLog()->Log("CreateSkeletonComponent failed.  Error: " + err);
	}

	StartDevice();

	return vr::VRInitError_None;
}

//This could do with a rename, its a bit vague as to what it does
void ControllerDriver::StartDevice() {

	m_communicationManager = std::make_unique<SerialManager>();

	m_communicationManager->Connect();

	if (m_communicationManager->IsConnected()) {

		m_communicationManager->BeginListener([&](const float* datas) {
			/*proposed structure for serial data
			0: pinky (range 0-analog_cap)
			1: ring  (range 0-analog_cap)
			2: middle (range 0-analog_cap)
			3: index (range 0-analog_cap)
			4: thumb (range 0-analog_cap)
			5: grab (0-1)						//I believe grab+pinch gestures should be determined by the arduino as this is where calibration takes place.
			6: pinch (0-1)						//This also allows for grab/pinch buttons to be used optionally as a substitute for estimated gestures.
			7: joyX (range 0-analog_cap)		//however this does mean 6 extra bytes of data that could have been calculated off board instead.
			8: joyY (range 0-analog_cap)
			9: button1 (0-1)
			10: button2 (0-1)
			*/
			//datas is a float array containing 0.0 - 1.0 floats that represent the extension of each finger (or joystick which will need to be scaled to -1.0 - 1.0)
			//and 0 || 1 for buttons
			float fingerFlexion[5] = { datas[0], datas[1], datas[2], datas[3], datas[4] };
			float fingerSplay[5] = { 0.5, 0.5, 0.5, 0.5, 0.5 };
			vr::VRBoneTransform_t handTransforms[NUM_BONES];
			ComputeEntireHand(handTransforms, fingerFlexion, fingerSplay, isRightHand());

			vr::EVRInputError err = vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, handTransforms, NUM_BONES);
			if (err != vr::VRInputError_None)
			{
				// Handle failure case TODO: switch to using driverlog.cpp
				vr::VRDriverLog()->Log("UpdateSkeletonComponent failed.  Error: " + err);
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
	//placeholder
	vr::DriverPose_t pose = { 0 };
	pose.poseIsValid = false;
	pose.result = vr::TrackingResult_Calibrating_OutOfRange;
	pose.deviceIsConnected = true;
	return pose;

	//for the pose, we can start a separate thread in PoseTracker.cpp which sends position data in a callback similar to how we handle comms.
	//Perhaps GetPose(), if needed for anything, just returns a DriverPose_t value from the last returned position saved in the callback.
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