#include "physical_device_provider.h"

vr::EVRInitError PhysicalDeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  return vr::VRInitError_None;
}

const char* const* PhysicalDeviceProvider::GetInterfaceVersions() {
  return vr::k_InterfaceVersions;
}

void PhysicalDeviceProvider::RunFrame() {
  // do nothing
}

void PhysicalDeviceProvider::EnterStandby() {}

void PhysicalDeviceProvider::LeaveStandby() {}

bool PhysicalDeviceProvider::ShouldBlockStandbyMode() {
  return false;
}

void PhysicalDeviceProvider::Cleanup() {}