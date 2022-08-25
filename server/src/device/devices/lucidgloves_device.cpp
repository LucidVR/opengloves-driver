#include "devices/lucidgloves_device.h"

#include "services/output/output_osc.h"

using namespace og;

LucidglovesDevice::LucidglovesDevice(const DeviceInfoData &device_info, std::unique_ptr<ICommunicationManager> communication_manager)
    : device_info_(device_info), communication_manager_(std::move(communication_manager)) {
  OutputOSCServer::GetInstance();
};

void LucidglovesDevice::ListenForInput(std::function<void(const og::InputPeripheralData &)> callback) {
  callback_ = callback;

  communication_manager_->BeginListener([&](Input data) {
    if (data.type == kInputDataType_Peripheral) {
      callback_(data.data.peripheral);

      OutputOSCServer::GetInstance().Send(device_info_.hand, data.data.peripheral);
    }
  });
}

og::DeviceInfoData LucidglovesDevice::GetInfo() {
  return device_info_;
}

LucidglovesDevice::~LucidglovesDevice() {
  OutputOSCServer::GetInstance().Stop();
}