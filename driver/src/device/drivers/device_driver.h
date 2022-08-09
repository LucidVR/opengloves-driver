#pragma once

#include <string>

#include "openvr_driver.h"

#include "opengloves_interface.h"

struct DeviceDriverInfo {
  std::string serial_number;
  vr::ETrackedControllerRole role;
};

class DeviceDriver : public vr::ITrackedDeviceServerDriver {
 public:
  virtual vr::EVRInitError Activate(uint32_t unObjectId) = 0;

  void Deactivate() override;

  void EnterStandby() override;

  void *GetComponent(const char *pchComponentNameAndVersion) override;

  void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize) override;

  vr::DriverPose_t GetPose() override;

  private:
  virtual void SetupProperties(vr::PropertyContainerHandle_t& props) = 0;
  virtual void SetupComponents(vr::PropertyContainerHandle_t& props) = 0;

  virtual void HandleInput(const og::InputPeripheralData& data) = 0;
};