#include <DeviceProvider.h>

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
	vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
	if (initError != vr::EVRInitError::VRInitError_None) {
		return initError;
	}
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(vr::VRDriverLog());
	DebugDriverLog("Initializing LucidGloves");

	VRDeviceConfiguration_t leftConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_LeftHand);
	VRDeviceConfiguration_t rightConfiguration = GetDeviceConfiguration(vr::TrackedControllerRole_RightHand);

	if (leftConfiguration.enabled) {
		m_leftHand = InstantiateDeviceDriver(leftConfiguration);
		vr::VRServerDriverHost()->TrackedDeviceAdded(m_leftHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_leftHand.get());
	}
	if (rightConfiguration.enabled) {
		m_rightHand = InstantiateDeviceDriver(rightConfiguration);
		vr::VRServerDriverHost()->TrackedDeviceAdded(m_rightHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_rightHand.get());
	}

	return vr::VRInitError_None;
}

std::unique_ptr<IDeviceDriver> DeviceProvider::InstantiateDeviceDriver(VRDeviceConfiguration_t configuration) {

	std::unique_ptr<ICommunicationManager> communicationManager;
	std::unique_ptr<IEncodingManager> encodingManager;

	bool isRightHand = configuration.role == vr::TrackedControllerRole_RightHand;

	switch (configuration.encodingProtocol) {
	default:
		DriverLog("No encoding protocol set. Using legacy.");
	case VREncodingProtocol::LEGACY:
		const int maxAnalogValue = vr::VRSettings()->GetInt32("encoding_legacy", "max_analog_value");
		encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
		break;
	}

	switch (configuration.communicationProtocol) {
	default:
		DriverLog("No communication protocol set. Using serial.");
	case VRCommunicationProtocol::SERIAL:
		char port[16];
		vr::VRSettings()->GetString("communication_serial", isRightHand ? "right_port" : "left_port", port, sizeof(port));
		VRSerialConfiguration_t serialSettings(port);

		communicationManager = std::make_unique<SerialCommunicationManager>(serialSettings, std::move(encodingManager));
		break;
	}

	switch (configuration.deviceDriver) {
	case VRDeviceDriver::EMULATED_KNUCKLES:
	{
		char serialNumber[32];
		vr::VRSettings()->GetString("device_knuckles", isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof(serialNumber));

		return std::make_unique<KnuckleDeviceDriver>(configuration, std::move(communicationManager), serialNumber);
	}

	default:
		DriverLog("No device driver selected. Using lucidgloves.");
	case VRDeviceDriver::LUCIDGLOVES:
	{
		char serialNumber[32];
		vr::VRSettings()->GetString("device_lucidgloves", isRightHand ? "right_serial_number" : "left_serial_number", serialNumber, sizeof(serialNumber));

		return std::make_unique<LucidGloveDeviceDriver>(configuration, std::move(communicationManager), serialNumber);
	}
	}

}
VRDeviceConfiguration_t DeviceProvider::GetDeviceConfiguration(vr::ETrackedControllerRole role) {
	const VRCommunicationProtocol communicationProtocol = (VRCommunicationProtocol)vr::VRSettings()->GetInt32(c_driverSettingsSection, "communication_protocol");
	const VREncodingProtocol encodingProtocol = (VREncodingProtocol)vr::VRSettings()->GetInt32(c_driverSettingsSection, "encoding_protocol");
	const VRDeviceDriver deviceDriver = (VRDeviceDriver)vr::VRSettings()->GetInt32(c_driverSettingsSection, "device_driver");


	const float offsetX = vr::VRSettings()->GetFloat(c_poseSettingsSection, "x_offset");
	const float offsetY = vr::VRSettings()->GetFloat(c_poseSettingsSection, "y_offset");
	const float offsetZ = vr::VRSettings()->GetFloat(c_poseSettingsSection, "z_offset");

	const float offsetXDeg = vr::VRSettings()->GetFloat(c_poseSettingsSection, "x_offset_degrees");
	const float offsetYDeg = vr::VRSettings()->GetFloat(c_poseSettingsSection, "y_offset_degrees");
	const float offsetZDeg = vr::VRSettings()->GetFloat(c_poseSettingsSection, "z_offset_degrees");

	const bool leftFlippedXPos = vr::VRSettings()->GetBool(c_poseSettingsSection, "left_flipped_pos_x");
	const bool leftFlippedYPos = vr::VRSettings()->GetBool(c_poseSettingsSection, "left_flipped_pos_y");
	const bool leftFlippedZPos = vr::VRSettings()->GetBool(c_poseSettingsSection, "left_flipped_pos_z");

	const bool leftFlippedXRot = vr::VRSettings()->GetBool(c_poseSettingsSection, "left_flipped_rot_x");
	const bool leftFlippedYRot = vr::VRSettings()->GetBool(c_poseSettingsSection, "left_flipped_rot_y");
	const bool leftFlippedZRot = vr::VRSettings()->GetBool(c_poseSettingsSection, "left_flipped_rot_z");

	const bool isRightHand = role == vr::TrackedControllerRole_RightHand;

	const bool isEnabled = vr::VRSettings()->GetBool(c_driverSettingsSection, isRightHand ? "right_enabled" : "left_enabled");

	//x axis may be flipped for the different hands
	const vr::HmdVector3_t offsetVector = {
		(isRightHand || !leftFlippedXPos) ? offsetX : -offsetX,
		(isRightHand || !leftFlippedYPos) ? offsetY : -offsetX,
		(isRightHand || !leftFlippedZPos) ? offsetZ : -offsetZ
	};
	const vr::HmdVector3_t angleOffsetVector = {
		(isRightHand || !leftFlippedXRot) ? offsetXDeg : -offsetXDeg,
		(isRightHand || !leftFlippedYRot) ? offsetYDeg : -offsetYDeg,
		(isRightHand || !leftFlippedZRot) ? offsetZDeg : -offsetZDeg
	};

	const float poseOffset = vr::VRSettings()->GetFloat(c_poseSettingsSection, "pose_offset");

	return VRDeviceConfiguration_t(role, isEnabled, offsetVector, angleOffsetVector, poseOffset, encodingProtocol, communicationProtocol, deviceDriver);

}
void DeviceProvider::Cleanup() {}
const char* const* DeviceProvider::GetInterfaceVersions() {
	return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame() {
	if (m_leftHand && m_leftHand->IsActive())
		m_leftHand->RunFrame();
	if (m_rightHand && m_rightHand->IsActive())
		m_rightHand->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode() {
	return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}
