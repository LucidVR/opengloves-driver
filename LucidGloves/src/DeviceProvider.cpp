#include <DeviceProvider.h>

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* pDriverContext)
{
	vr::EVRInitError initError = InitServerDriverContext(pDriverContext);
	if (initError != vr::EVRInitError::VRInitError_None)
	{
		return initError;
	}

	DebugDriverLog("Initializing LucidGloves");

	VRDeviceConfiguration_t leftConfiguration = GetConfiguration(vr::TrackedControllerRole_LeftHand);
	VRDeviceConfiguration_t rightConfiguration = GetConfiguration(vr::TrackedControllerRole_RightHand);

	m_leftHand = new ControllerDriver(leftConfiguration);
	m_rightHand = new ControllerDriver(rightConfiguration);

	vr::VRServerDriverHost()->TrackedDeviceAdded(c_leftControllerSerialNumber, vr::TrackedDeviceClass_Controller, m_leftHand);
	vr::VRServerDriverHost()->TrackedDeviceAdded(c_rightControllerSerialNumber, vr::TrackedDeviceClass_Controller, m_rightHand);

	return vr::VRInitError_None;
}
VRDeviceConfiguration_t DeviceProvider::GetConfiguration(vr::ETrackedControllerRole role) {
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

			VRSerialConfiguration_t serialSettings(port);
			VRDeviceConfiguration_t deviceSettings(role, offsetVector, poseOffset, serialSettings);

			return deviceSettings;
		}
}
void DeviceProvider::Cleanup()
{
	delete m_leftHand;
	delete m_rightHand;

	m_leftHand = nullptr;
	m_rightHand = nullptr;
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
	return vr::k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
	return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}