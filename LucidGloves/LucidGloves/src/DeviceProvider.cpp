#include <DeviceProvider.h>

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
    EVRInitError initError = InitServerDriverContext(pDriverContext);
    if (initError != EVRInitError::VRInitError_None)
    {
        return initError;
    }
    
   DebugDriverLog("Initializing LucidGloves");

    std::unique_ptr<ControllerDriver> m_leftHand = std::make_unique<ControllerDriver>(TrackedControllerRole_LeftHand);
    std::unique_ptr<ControllerDriver> m_rightHand = std::make_unique<ControllerDriver>(TrackedControllerRole_RightHand);

    VRServerDriverHost()->TrackedDeviceAdded(c_leftControllerSerialNumber, TrackedDeviceClass_Controller, m_leftHand.get());
    VRServerDriverHost()->TrackedDeviceAdded(c_rightControllerSerialNumber, TrackedDeviceClass_Controller, m_rightHand.get());

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