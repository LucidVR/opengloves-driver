// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "prober_serial_connectable.h"

#include "communication/services/service_serial.h"
#include "opengloves_interface.h"

SerialPortProber::SerialPortProber(const SerialPortProberConfiguration& configuration) {
  port_ = configuration.port;
}

bool SerialPortProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) {
  og::DeviceSerialCommunicationConfiguration config{port_};
  auto device = std::make_unique<SerialCommunicationService>(config);
  if (device->IsConnected()) {
    out_devices.push_back(std::move(device));
    return true;
  }

  return false;
}