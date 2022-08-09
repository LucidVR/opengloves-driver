#include "device_driver.h"

#include "driver_log.h"


vr::EVRInitError DeviceDriver::Activate(uint32_t unObjectId) {

  return vr::VRInitError_None;
}



void* DeviceDriver::GetComponent(const char* pchComponentNameAndVersion) {
  return nullptr;
}

void DeviceDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, const uint32_t unResponseBufferSize) {
  if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0;
}

vr::DriverPose_t DeviceDriver::GetPose() {
  DriverLog("Get pose called for knuckle device driver. Returning empty pose.");

  vr::DriverPose_t pose{};
  return pose;
}
void DeviceDriver::EnterStandby() {}

void DeviceDriver::Deactivate() {}