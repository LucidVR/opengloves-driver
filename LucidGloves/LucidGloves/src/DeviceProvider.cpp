#include <DeviceProvider.h>

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
    EVRInitError initError = InitServerDriverContext(pDriverContext);
    if (initError != EVRInitError::VRInitError_None)
    {
        return initError;
    }
    
    VRDriverLog()->Log("Initializing LucidGloves"); //this is how you log out Steam's log file.

    leftHand = new ControllerDriver();
    rightHand = new ControllerDriver();

    leftHand->Init(TrackedControllerRole_LeftHand);
    rightHand->Init(TrackedControllerRole_LeftHand);

    VRServerDriverHost()->TrackedDeviceAdded("lucidgloves-left", TrackedDeviceClass_Controller, leftHand);
    VRServerDriverHost()->TrackedDeviceAdded("lucidgloves-right", TrackedDeviceClass_Controller, rightHand);

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup()
{
    delete leftHand;
    leftHand = NULL;
    delete rightHand;
    rightHand = NULL;
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
    return k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
    leftHand->RunFrame();
    rightHand->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
    return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}