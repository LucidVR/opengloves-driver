#include "devices/lucidgloves_device.h"

using namespace og;

LucidglovesDevice::LucidglovesDevice(const DeviceInfoData &device_info, std::unique_ptr<CommunicationManager> communication_manager)
    : device_info_(device_info), communication_manager_(std::move(communication_manager)){};

void LucidglovesDevice::ListenForInput(std::function<void(og::InputPeripheralData)> &callback) {
  callback_ = callback;

  communication_manager_->BeginListener([&](Input data) {
    if (data.type == kInputDataType_Peripheral) {
      callback_(data.data.peripheral);
    }
  });
}

og::DeviceInfoData LucidglovesDevice::GetInfo() {
  return device_info_;
}