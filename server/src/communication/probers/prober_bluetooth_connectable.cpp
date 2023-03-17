// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "prober_bluetooth_connectable.h"

#include <utility>

#include "communication/services/service_bluetooth.h"

BluetoothPortProber::BluetoothPortProber(BluetoothPortProberConfiguration configuration) : configuration_(std::move(configuration)) {}

bool BluetoothPortProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>> &out_devices) {
  og::DeviceBluetoothCommunicationConfiguration service_configuration{configuration_.port};
  std::unique_ptr<BluetoothCommunicationService> bluetooth_service = std::make_unique<BluetoothCommunicationService>(service_configuration);

  if (bluetooth_service->IsConnected()) {
    out_devices.emplace_back(std::move(bluetooth_service));

    return true;
  }

  return false;
}