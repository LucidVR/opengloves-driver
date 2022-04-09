#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "DeviceConfiguration.h"
#include "DeviceDriver/DeviceDriver.h"
#include "openvr_driver.h"

class DeviceDriverManager : public vr::ITrackedDeviceServerDriver {
 public:
  DeviceDriverManager(std::unique_ptr<DeviceDriver> device) : device_(std::move(device)){};

  vr::EVRInitError Activate(uint32_t unObjectId) override;
  void Deactivate() override;
  void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
  void EnterStandby() override;
  void* GetComponent(const char* pchComponentNameAndVersion) override;
  vr::DriverPose_t GetPose() override;

  virtual std::string GetSerialNumber();
  int32_t GetDeviceId() const;

  virtual bool IsActive();

  void OnEvent(vr::VREvent_t vrEvent) const;
  void SetDeviceDriver(std::unique_ptr<DeviceDriver> newDevice);

 private:
  std::unique_ptr<DeviceDriver> device_;

  int32_t deviceId_ = -1;
  std::atomic<bool> isActive_ = false;
}; 