#include "device_provider.h"

#include "util/driver_log.h"

vr::EVRInitError DeviceProvider::Init(vr::IVRDriverContext* context) {
  const vr::EVRInitError err = vr::InitServerDriverContext(context);
  if (err != vr::VRInitError_None) return err;

  InitDriverLog(vr::VRDriverLog());

  DebugDriverLog("opengloves is running in debug");

  // create a probers for device



  return vr::VRInitError_None;
}

void DeviceProvider::RunFrame() {}

void DeviceProvider::EnterStandby() {}

void DeviceProvider::LeaveStandby() {}

void DeviceProvider::Cleanup() {}

bool DeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

const char* const* DeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}
