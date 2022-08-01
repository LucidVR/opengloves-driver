#pragma once

#include <memory>

#include "opengloves_interface.h"

#include "managers/communication_manager.h"

class LucidglovesDevice : public og::Device {
 public:
  LucidglovesDevice(const og::DeviceInfoData& device_info, std::unique_ptr<CommunicationManager> communication_manager);

  og::DeviceInfoData GetInfo() override;
  void ListenForInput(std::function<void(og::InputPeripheralData data)>& callback) override;

 private:
  std::function<void(og::InputPeripheralData)> callback_;

  og::DeviceInfoData device_info_;

  std::unique_ptr<CommunicationManager> communication_manager_;
};