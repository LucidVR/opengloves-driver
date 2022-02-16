#include <memory>

#include "DeviceConfiguration.h"

#include "DeviceProvider/DeviceProvider.h"
#include "DeviceProvider/HookingDeviceProvider.h"
#include "DeviceProvider/PhysicalDeviceProvider.h"

#include "DriverLog.h"
#include "openvr_driver.h"

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

std::unique_ptr<DeviceProvider> g_DeviceProvider;

/**
This method returns an instance of your provider that OpenVR uses.
**/
HMD_DLL_EXPORT
void* HmdDriverFactory(const char* interfaceName, int* returnCode) {
  if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, interfaceName)) {
    VRDeviceProvider deviceProvider = static_cast<VRDeviceProvider>(vr::VRSettings()->GetInt32(c_driverSettingsSection, "device_provider"));

    switch (deviceProvider) {
      case VRDeviceProvider::HookingDeviceProvier:
        DriverLog("Using Hooking Device Provider");
        g_DeviceProvider = std::make_unique<HookingDeviceProvider>();
        break;
      case VRDeviceProvider::PhysicalDeviceProvider:
      default:
        DriverLog("Using Physical Device Provider");
        g_DeviceProvider = std::make_unique<PhysicalDeviceProvider>();
        break;
    }
    return g_DeviceProvider.get();
  }
  DriverLog("HmdDriverFactory called for %s", interfaceName);
  return nullptr;
}