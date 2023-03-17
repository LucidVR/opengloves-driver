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

struct SerialProberIdentifier {
  std::string vid;
  std::string pid;
};

class SerialIdentifierProber : public ICommunicationProber {
 public:
  explicit SerialIdentifierProber(SerialProberIdentifier identifier);

  bool InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;
 private:
  std::string identifier_;
};