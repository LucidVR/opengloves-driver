#include <DeviceProvider.h>

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
    EVRInitError initError = InitServerDriverContext(pDriverContext);
    if (initError != EVRInitError::VRInitError_None)
    {
        return initError;
    }
    
    VRDriverLog()->Log("Initializing LucidGloves"); //this is how you log out Steam's log file.

    m_leftHand = new ControllerDriver();
    m_rightHand = new ControllerDriver();

    m_leftHand->Init(TrackedControllerRole_LeftHand);
    m_rightHand->Init(TrackedControllerRole_RightHand);

    VRServerDriverHost()->TrackedDeviceAdded(left_controller_serial, TrackedDeviceClass_Controller, m_leftHand);
    VRServerDriverHost()->TrackedDeviceAdded(right_controller_serial, TrackedDeviceClass_Controller, m_rightHand);

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup()
{
    delete m_leftHand;
    m_leftHand = NULL;
    delete m_rightHand;
    m_rightHand = NULL;
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