#pragma once

#include "openvr_driver.h"
#include "opengloves_interface.h"

class IDeviceDriver : public vr::ITrackedDeviceServerDriver {
 public:
  virtual bool IsActive() = 0;
  virtual std::string GetSerialNumber() = 0;

  virtual void SetDeviceDriver(std::unique_ptr<og::IDevice>) = 0;
};