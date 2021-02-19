#include <DeviceProvider.h>

EVRInitError DeviceProvider::Init(IVRDriverContext* pDriverContext)
{
    EVRInitError initError = InitServerDriverContext(pDriverContext);
    if (initError != EVRInitError::VRInitError_None)
    {
        return initError;
    }
    
    VRDriverLog()->Log("Initializing LucidGloves"); //this is how you log out Steam's log file.

    controllerDriver = new ControllerDriver();
    VRServerDriverHost()->TrackedDeviceAdded("lucidgloves", TrackedDeviceClass_Controller, controllerDriver); //add all your devices like this.

    return vr::VRInitError_None;
}

void DeviceProvider::Cleanup()
{
    delete controllerDriver;
    controllerDriver = NULL;
}
const char* const* DeviceProvider::GetInterfaceVersions()
{
    return k_InterfaceVersions;
}

void DeviceProvider::RunFrame()
{
    controllerDriver->RunFrame();
}

bool DeviceProvider::ShouldBlockStandbyMode()
{
    return false;
}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}