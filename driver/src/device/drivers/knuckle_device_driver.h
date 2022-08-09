#pragma once

#include <string>

#include "device_driver.h"

class KnuckleDeviceDriver : public DeviceDriver {
 private:
  KnuckleDeviceDriver(const std::string& serial_number);

  void SetupProperties(vr::PropertyContainerHandle_t& props) override;
  void SetupComponents(vr::PropertyContainerHandle_t& props) override;

  void HandleInput(const og::InputPeripheralData& data) override;

 private:
  std::string serial_number_;
};