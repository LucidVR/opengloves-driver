// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "communication/probers/prober.h"
#include "communication/services/communication_service.h"

struct SerialPortProberConfiguration {
  std::string port;
};

class SerialPortProber : public ICommunicationProber {
 public:
  explicit SerialPortProber(const SerialPortProberConfiguration& configuration);

  bool InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

 private:
  std::string port_;
};