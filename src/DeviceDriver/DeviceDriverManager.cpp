#include "DeviceDriver/DeviceDriverManager.h"

#include "DriverLog.h"

vr::EVRInitError DeviceDriverManager::Activate(uint32_t unObjectId) {
  deviceId_ = unObjectId;

  device_->Activate(unObjectId);

  isActive_ = true;
  return vr::VRInitError_None;
}

void DeviceDriverManager::DeactivateDeviceDriver() {
  if (device_ == nullptr) return;

  device_->Deactivate();
}

void DeviceDriverManager::Deactivate() {
  DeactivateDeviceDriver();

  isActive_ = false;
}

void DeviceDriverManager::DebugRequest(const char* pchRequest, char* pchResponseBuffer, const uint32_t unResponseBufferSize) {}

void DeviceDriverManager::EnterStandby() {}

void* DeviceDriverManager::GetComponent(const char* pchComponentNameAndVersion) {
  return nullptr;
}

vr::DriverPose_t DeviceDriverManager::GetPose() {
  if (isActive_) return device_->GetPose();

  return vr::DriverPose_t{0};
}

std::string DeviceDriverManager::GetSerialNumber() {
  return device_->GetSerialNumber();
}

int32_t DeviceDriverManager::GetDeviceId() const {
  return deviceId_;
}

bool DeviceDriverManager::IsActive() {
  return isActive_;
}

void DeviceDriverManager::OnEvent(vr::VREvent_t vrEvent) const {
  device_->OnEvent(vrEvent);
}

void DeviceDriverManager::SetDeviceDriver(std::unique_ptr<DeviceDriver> newDevice) {
  if (device_->IsActive()) device_->Deactivate();

  if (isActive_) newDevice->Activate(deviceId_);

  device_ = std::move(newDevice);

  DriverLog("Successfully updated device driver");
}
