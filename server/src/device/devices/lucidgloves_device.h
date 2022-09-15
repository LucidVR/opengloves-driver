#pragma once

#include <memory>

#include "opengloves_interface.h"

#include "communication/managers/communication_manager.h"

class LucidglovesDevice : public og::Device {
 public:
  LucidglovesDevice(og::DeviceConfiguration configuration, std::unique_ptr<ICommunicationManager> communication_manager);

  og::DeviceConfiguration GetConfiguration() override;
  void ListenForInput(std::function<void(const og::InputPeripheralData& data)> callback) override;

  void Output(const og::Output& output) override;

  ~LucidglovesDevice();

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  og::DeviceConfiguration configuration_;

};