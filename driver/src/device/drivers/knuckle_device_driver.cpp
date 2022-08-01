#include "knuckle_device_driver.h"

#include "driver_log.h"

vr::EVRInitError KnuckleDeviceDriver::Activate(uint32_t unObjectId) {
  return vr::VRInitError_None;
}

vr::DriverPose_t KnuckleDeviceDriver::GetPose() {
  DriverLog("Get pose called for knuckle device driver. Returning empty pose.");

  vr::DriverPose_t pose{};
  return pose;
}

void* KnuckleDeviceDriver::GetComponent(const char* pchComponentNameAndVersion) {
  return nullptr;
}

void KnuckleDeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, const uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0;
}

void KnuckleDeviceDriver::EnterStandby() {}

void KnuckleDeviceDriver::Deactivate() {}