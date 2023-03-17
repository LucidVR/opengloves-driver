// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <memory>

#include "opengloves_interface.h"

#include "communication/managers/communication_manager.h"

class LucidglovesDevice : public og::IDevice {
 public:
  LucidglovesDevice(og::DeviceConfiguration configuration, std::unique_ptr<ICommunicationManager> communication_manager);

  og::DeviceConfiguration GetConfiguration() override;
  void ListenForInput(std::function<void(const og::InputPeripheralData& data)> callback) override;

  void Output(const og::Output& output) override;

  ~LucidglovesDevice() override;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  og::DeviceConfiguration configuration_;

};