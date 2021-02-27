#include <DeviceProvider.h>

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext)
{
	vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
	if (initError != vr::EVRInitError::VRInitError_None)
	{
		return initError;
	}

	DebugDriverLog("Initializing LucidGloves");

	VRDeviceConfiguration leftConfiguration = GetConfiguration(vr::TrackedControllerRole_LeftHand);
	VRDeviceConfiguration rightConfiguration = GetConfiguration(vr::TrackedControllerRole_RightHand);

	std::unique_ptr<ControllerDriver> m_leftHand = std::make_unique<ControllerDriver>(leftConfiguration);
	std::unique_ptr<ControllerDriver> m_rightHand = std::make_unique<ControllerDriver>(rightConfiguration);

	vr::VRServerDriverHost()->TrackedDeviceAdded(c_leftControllerSerialNumber, vr::TrackedDeviceClass_Controller, m_leftHand.get());
	vr::VRServerDriverHost()->TrackedDeviceAdded(c_rightControllerSerialNumber, vr::TrackedDeviceClass_Controller, m_rightHand.get());

	return vr::VRInitError_None;
}
VRDeviceConfiguration DeviceProvider::GetConfiguration(vr::ETrackedControllerRole role) {
	const int protocol = vr::VRSettings()->GetInt32(c_settingsSection, "protocol");

	const float offsetX = vr::VRSettings()->GetFloat(c_settingsSection, "x_offset");
	const float offsetY = vr::VRSettings()->GetFloat(c_settingsSection, "y_offset");
	const float offsetZ = vr::VRSettings()->GetFloat(c_settingsSection, "z_offset");

	const vr::HmdVector3_t offsetVector = { offsetX, offsetY, offsetZ };

	const float poseOffset = vr::VRSettings()->GetFloat(c_settingsSection, "pose_offset");


	switch (protocol) {
		case VRDeviceProtocol::SERIAL:
			char port[64];
			vr::VRSettings()->GetString(c_settingsSection, role == vr::TrackedControllerRole_RightHand ? "right_port" : "left_port", port, sizeof(port));

			VRSerialConfiguration serialSettings(port);
			VRDeviceConfiguration deviceSettings(role, offsetVector, poseOffset, serialSettings);

			return deviceSettings;
		}
}
void DeviceProvider::Cleanup()
{
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
	return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
	m_leftHand->RunFrame();
	m_rightHand->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
	return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}