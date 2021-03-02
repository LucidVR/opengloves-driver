#include <DeviceProvider.h>
#include <openvr_driver.h>
#include <windows.h>

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform."
#endif

DeviceProvider deviceProvider; //global, single instance, of the class that provides OpenVR with all of your devices.

/**
This method returns an instance of your provider that OpenVR uses.
**/
HMD_DLL_EXPORT
void* HmdDriverFactory(const char* interfaceName, int* returnCode)
{

	if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, interfaceName))
	{
		return &deviceProvider;
	}
	DriverLog("HmdDriverFactory called for %s", interfaceName);
	return nullptr;
}