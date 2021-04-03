#include <DeviceProvider.h>

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
	vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
	if (initError != vr::EVRInitError::VRInitError_None) {
		return initError;
	}
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	InitDriverLog(vr::VRDriverLog());
	DebugDriverLog("Initializing LucidGloves");

	std::unique_ptr<VRDeviceConfiguration_t> leftConfiguration = GetConfiguration(vr::TrackedControllerRole_LeftHand);
	std::unique_ptr<VRDeviceConfiguration_t> rightConfiguration = GetConfiguration(vr::TrackedControllerRole_RightHand);

	if (leftConfiguration->enabled) {
		m_leftHand = InstantiateDeviceDriver(std::move(leftConfiguration));
		vr::VRServerDriverHost()->TrackedDeviceAdded(m_leftHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_leftHand.get());
	}
	if (rightConfiguration->enabled) {
		m_rightHand = InstantiateDeviceDriver(std::move(rightConfiguration));
		vr::VRServerDriverHost()->TrackedDeviceAdded(m_rightHand->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_rightHand.get());
	}

	return vr::VRInitError_None;
}

std::unique_ptr<IDeviceDriver> DeviceProvider::InstantiateDeviceDriver(std::unique_ptr<VRDeviceConfiguration_t> configuration) {
	switch (configuration->selectedDeviceDriver) {
	case VRDeviceDriver::EMULATED_KNUCKLES:
		return std::make_unique<KnuckleDeviceDriver>(std::move(configuration));
	default:
		DriverLog("No device driver selected. Using lucidgloves.");
	case VRDeviceDriver::LUCIDGLOVES:
		return std::make_unique<LucidGloveDeviceDriver>(std::move(configuration));
	}
}
std::unique_ptr<VRDeviceConfiguration_t> DeviceProvider::GetConfiguration(vr::ETrackedControllerRole role) {
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

	std::unique_ptr<ICommunicationManager> communicationManager;
	std::unique_ptr<IEncodingManager> encodingManager;


	switch (encodingProtocol) {
	default:
		DriverLog("No encoding protocol set. Using legacy.");
	case VREncodingProtocol::LEGACY:
		const int maxAnalogValue = vr::VRSettings()->GetInt32("encoding_legacy", "max_analog_value");
		encodingManager = std::make_unique<LegacyEncodingManager>(maxAnalogValue);
		break;
	}

	switch (communicationProtocol) {
	default:
		DriverLog("No communication protocol set. Using serial.");
	case VRCommunicationProtocol::SERIAL:
		char port[16];
		vr::VRSettings()->GetString("communication_serial", role == vr::TrackedControllerRole_RightHand ? "right_port" : "left_port", port, sizeof(port));
		VRSerialConfiguration_t serialSettings(port);

		communicationManager = std::make_unique<SerialCommunicationManager>(serialSettings, std::move(encodingManager));
		break;
	}

	switch (deviceDriver) {
	case VRDeviceDriver::EMULATED_KNUCKLES:
	{
		char buffer[32];
		vr::VRSettings()->GetString("device_knuckles", role == vr::TrackedControllerRole_RightHand ? "right_serial_number" : "left_serial_number", buffer, sizeof(buffer));

		return std::make_unique<VRDeviceConfiguration_t>(role, isEnabled, offsetVector, angleOffsetVector, poseOffset, std::move(communicationManager), deviceDriver, buffer);
	}

	default:
		DriverLog("No device driver selected. Using lucidgloves.");
	case VRDeviceDriver::LUCIDGLOVES:
	{
		char buffer[32];
		vr::VRSettings()->GetString("device_lucidgloves", role == vr::TrackedControllerRole_RightHand ? "right_serial_number" : "left_serial_number", buffer, sizeof(buffer));

		return std::make_unique<VRDeviceConfiguration_t>(role, isEnabled, offsetVector, angleOffsetVector, poseOffset, std::move(communicationManager), deviceDriver, buffer);
	}
	}

}
void DeviceProvider::Cleanup() {}
const char* const* DeviceProvider::GetInterfaceVersions() {
	return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame() {
	if (m_leftHand->IsActive())
		m_leftHand->RunFrame();
	if (m_rightHand->IsActive())
		m_rightHand->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode() {
	return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}
