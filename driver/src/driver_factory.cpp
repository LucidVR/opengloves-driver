// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include <cstring>

#include "device/providers/physical_device_provider.h"
#include "openvr_driver.h"
#include "util/driver_log.h"

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec(dllexport)
#define HMD_DLL_IMPORT extern "C" __declspec(dllimport)
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

PhysicalDeviceProvider physical_device_provider;  // only physical devices supported right now

HMD_DLL_EXPORT
void* HmdDriverFactory(const char* interfaceName, int* returnCode) {
  if (strcmp(vr::IServerTrackedDeviceProvider_Version, interfaceName) == 0) {
    return &physical_device_provider;
  }

  DriverLog("HmdDriverFactory called for %s", interfaceName);
  return nullptr;
}