#include "physical_device_provider.h"

#include "device_configuration/device_configuration.h"
#include "util/driver_log.h"

vr::EVRInitError PhysicalDeviceProvider::Init(vr::IVRDriverContext* pDriverContext) {
  const vr::EVRInitError err = vr::InitServerDriverContext(pDriverContext);

  if (err != vr::VRInitError_None) {
    DriverLog("Failed to initialise server driver context! Error: %i", err);

    return err;
  }

  if (!InitDriverLog(vr::VRDriverLog())) {
    DriverLog("Failed to initialise driver logs!");

    return vr::VRInitError_Driver_NotLoaded;
  }

  DebugDriverLog("OpenGloves is running in DEBUG MODE");

  // initialise opengloves
 // ogserver_ = std::make_unique<og::Server>();

 // ogserver_->SetLegacyConfiguration(GetDriverLegacyConfiguration(vr::TrackedControllerRole_LeftHand));

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