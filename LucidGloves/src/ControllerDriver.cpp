#include "ControllerDriver.h"

ControllerDriver::ControllerDriver(const VRDeviceConfiguration_t &configuration)
	: m_configuration(configuration), m_driverId(-1), m_hasActivated(false) {

	//copy a default bone transform to our hand transform for use in finger positioning later
	std::copy(
		std::begin(m_configuration.role == vr::TrackedControllerRole_RightHand ? rightOpenPose : leftOpenPose),
		std::end(m_configuration.role == vr::TrackedControllerRole_RightHand ? rightOpenPose : leftOpenPose),
		std::begin(m_handTransforms)
	);


	switch (m_configuration.communicationProtocol) {
	default:
		DriverLog("No communication protocol defined... using default.");
	case VRCommunicationProtocol::SERIAL:
		m_communicationManager = std::make_unique<SerialManager>(m_configuration.serialConfiguration);
		break;
	}
	switch (m_configuration.encodingProtocol) {
	default:
		DriverLog("No encoding protocol defined... using default.");
	case VREncodingProtocol::LEGACY:
		m_encodingManager = std::make_unique<LegacyEncodingManager>((float)m_configuration.maxAnalogValue);
		break;
	}
}

bool ControllerDriver::IsRightHand() const {
	return m_configuration.role == vr::TrackedControllerRole_RightHand;
}

vr::EVRInitError ControllerDriver::Activate(uint32_t unObjectId) {
	DebugDriverLog("Activating lucidgloves... ID: %d, role: %d, enabled: %s", unObjectId, m_configuration.role, m_configuration.enabled?"true":"false");
	const bool isRightHand = IsRightHand();

	m_driverId = unObjectId; //unique ID for your driver
	m_controllerPose = std::make_unique<ControllerPose>(m_configuration.role, std::string(c_deviceManufacturer), m_configuration, m_driverId);

	vr::PropertyContainerHandle_t props = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_driverId); //this gets a container object where you store all the information about your driver

	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerHandSelectionPriority_Int32, (int32_t)2147483647);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_InputProfilePath_String, c_inputProfilePath); //tell OpenVR where to get your driver's Input Profile
	vr::VRProperties()->SetInt32Property(props, vr::Prop_ControllerRoleHint_Int32, m_configuration.role); //tells OpenVR what kind of device this is
	vr::VRProperties()->SetStringProperty(props, vr::Prop_SerialNumber_String, isRightHand ? c_rightControllerSerialNumber : c_leftControllerSerialNumber);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ModelNumber_String, c_deviceModelNumber);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ManufacturerName_String, c_deviceManufacturer);
	vr::VRProperties()->SetInt32Property(props, vr::Prop_DeviceClass_Int32, (int32_t)vr::TrackedDeviceClass_Controller);
	vr::VRProperties()->SetStringProperty(props, vr::Prop_ControllerType_String, c_deviceControllerType);

	vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/x", &m_inputComponentHandles[ComponentIndex::COMP_JOY_X], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(props, "/input/joystick/y", &m_inputComponentHandles[ComponentIndex::COMP_JOY_Y], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/joystick/click", &m_inputComponentHandles[ComponentIndex::COMP_JOY_BTN]);

	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/trigger/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_TRG]);

	vr::VRDriverInput()->CreateBooleanComponent(props, isRightHand ? "/input/A/click" : "/input/system/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_A]);

	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/B/click", &m_inputComponentHandles[ComponentIndex::COMP_BTN_B]);

	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/grab/click", &m_inputComponentHandles[ComponentIndex::COMP_GES_GRAB]);
	vr::VRDriverInput()->CreateBooleanComponent(props, "/input/pinch/click", &m_inputComponentHandles[ComponentIndex::COMP_GES_PINCH]);

	vr::VRDriverInput()->CreateHapticComponent(props, "output/haptic", &m_inputComponentHandles[ComponentIndex::COMP_HAPTIC]);

	vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/thumb", &m_inputComponentHandles[ComponentIndex::COMP_TRG_THUMB], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/index", &m_inputComponentHandles[ComponentIndex::COMP_TRG_INDEX], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/middle", &m_inputComponentHandles[ComponentIndex::COMP_TRG_MIDDLE], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/ring", &m_inputComponentHandles[ComponentIndex::COMP_TRG_RING], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(props, "/input/finger/pinky", &m_inputComponentHandles[ComponentIndex::COMP_TRG_PINKY], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedOneSided);

	// Create the skeletal component and save the handle for later use

	vr::EVRInputError error = vr::VRDriverInput()->CreateSkeletonComponent(props,
		isRightHand ? "/input/skeleton/right" : "/input/skeleton/left",
		isRightHand ? "/skeleton/hand/right" : "/skeleton/hand/left",
		"/pose/raw",
		vr::VRSkeletalTracking_Partial,
		isRightHand ? rightOpenPose : leftOpenPose,
		NUM_BONES,
		&m_skeletalComponentHandle);

	if (error != vr::VRInputError_None) {
		// Handle failure case
		DebugDriverLog("CreateSkeletonComponent failed.  Error: %s\n", error);
	}

	StartDevice();

	m_hasActivated = true;

	return vr::VRInitError_None;
}

//This could do with a rename, its a bit vague as to what it does
void ControllerDriver::StartDevice() {
	m_communicationManager->Connect();
	//DebugDriverLog("Getting ready to connect:");
	if (m_communicationManager->IsConnected()) {
		//DebugDriverLog("Connected successfully");
		m_communicationManager->BeginListener([&](std::string receivedString) {
			//decode the receivedString
			try {
				VRCommData_t datas = m_encodingManager->Decode(receivedString);

				try {
					//Compute each finger transform
					for (int i = 0; i < NUM_BONES; i++) {
						int fingerNum = FingerFromBone(i);
						if (fingerNum != -1) {
							ComputeBoneFlexion(&m_handTransforms[i], datas.flexion[fingerNum], i, IsRightHand());
						}
					}
					vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithoutController, m_handTransforms, NUM_BONES);
					vr::VRDriverInput()->UpdateSkeletonComponent(m_skeletalComponentHandle, vr::VRSkeletalMotionRange_WithController, m_handTransforms, NUM_BONES);

					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_X], datas.joyX, 0);
					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_Y], datas.joyY, 0);

					vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_JOY_BTN], datas.joyButton, 0);
					vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_TRG], datas.trgButton, 0);
					vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_A], datas.aButton, 0);
					vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_BTN_B], datas.bButton, 0);

					vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_GES_GRAB], datas.grab, 0);
					vr::VRDriverInput()->UpdateBooleanComponent(m_inputComponentHandles[ComponentIndex::COMP_GES_PINCH], datas.pinch, 0);

					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_THUMB], datas.flexion[0], 0);
					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_INDEX], datas.flexion[1], 0);
					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_MIDDLE], datas.flexion[2], 0);
					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_RING], datas.flexion[3], 0);
					vr::VRDriverInput()->UpdateScalarComponent(m_inputComponentHandles[ComponentIndex::COMP_TRG_PINKY], datas.flexion[4], 0);
				}
				catch (const std::exception& e) {
					DebugDriverLog("Caught exception updating components");
				}

			}
			catch (const std::exception& e) {
				DebugDriverLog("Exception caught while parsing comm data");
			}
		});

	}
	else {
		DebugDriverLog("Device did not connect successfully");
		//Perhaps retry
	}
}

vr::DriverPose_t ControllerDriver::GetPose() {
	if(m_hasActivated) return m_controllerPose->UpdatePose();

	vr::DriverPose_t pose = { 0 };
	return pose;
}

void ControllerDriver::RunFrame() {
	if (m_hasActivated) {
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_driverId, m_controllerPose->UpdatePose(), sizeof(vr::DriverPose_t));
	}
}


void ControllerDriver::Deactivate() {
	if (m_hasActivated) {
		m_communicationManager->Disconnect();
		m_driverId = vr::k_unTrackedDeviceIndexInvalid;
	}
}

void* ControllerDriver::GetComponent(const char* pchComponentNameAndVersion) {
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
	if (unResponseBufferSize >= 1) {
		pchResponseBuffer[0] = 0;
	}
}