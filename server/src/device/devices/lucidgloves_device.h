#pragma once

#include <memory>

#include "opengloves_interface.h"

#include "communication/managers/communication_manager.h"

class LucidglovesDevice : public og::Device {
 public:
  LucidglovesDevice(const og::DeviceInfoData& device_info, std::unique_ptr<ICommunicationManager> communication_manager);

  og::DeviceInfoData GetInfo() override;
  void ListenForInput(std::function<void(const og::InputPeripheralData& data)> callback) override;

  ~LucidglovesDevice();

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;

  og::DeviceInfoData device_info_;

};