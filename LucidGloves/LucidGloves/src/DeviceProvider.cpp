#include <DeviceProvider.h>

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
    EVRInitError initError = InitServerDriverContext(pDriverContext);
    if (initError != EVRInitError::VRInitError_None)
    {
        return initError;
    }
    
    VRDriverLog()->Log("Initializing LucidGloves"); //this is how you log out Steam's log file.

    std::unique_ptr<ControllerDriver> m_leftHand = std::make_unique<ControllerDriver>(TrackedControllerRole_LeftHand);
    std::unique_ptr<ControllerDriver> m_rightHand = std::make_unique<ControllerDriver>(TrackedControllerRole_RightHand);

    VRServerDriverHost()->TrackedDeviceAdded(left_controller_serial, TrackedDeviceClass_Controller, m_leftHand.get());
    VRServerDriverHost()->TrackedDeviceAdded(right_controller_serial, TrackedDeviceClass_Controller, m_rightHand.get());

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup()
{
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
    return k_InterfaceVersions;
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