// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <string>
#include <vector>

#include "communication/probers/prober.h"
#include "opengloves_interface.h"

struct BluetoothProberConfiguration {
  std::string identifier;
};

class BluetoothCommunicationProber : public ICommunicationProber {
 public:
  explicit BluetoothCommunicationProber(BluetoothProberConfiguration configuration);

  bool InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

 private:
  BluetoothProberConfiguration configuration_;
};