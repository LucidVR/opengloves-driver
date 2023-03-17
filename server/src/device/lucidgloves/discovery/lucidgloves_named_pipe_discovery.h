// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <memory>

#include "opengloves_interface.h"

class LucidglovesNamedPipeDiscovery : public og::IDeviceDiscoverer {
 public:
  LucidglovesNamedPipeDiscovery();

  void StartDiscovery(std::function<void(std::unique_ptr<og::IDevice> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesNamedPipeDiscovery() override;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  std::function<void(std::unique_ptr<og::IDevice> device)> device_discovered_callback_;
};