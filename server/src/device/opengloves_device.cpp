#include "managers/communication_manager.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

Device::Device(DeviceInfoData info_data, std::unique_ptr<CommunicationManager> communication_manager) {
  communication_manager_ = std::move(communication_manager);

  info_data_ = info_data;
}

DeviceInfoData Device::GetInfo() {
  return info_data_;
}

void Device::ListenForInput(std::function<void(InputPeripheralData data)>& callback) {
  callback_ = callback;

  communication_manager_->BeginListener([&](Input data) {
    if (data.type == kInputDataType_Peripheral) {
      callback_(data.data.peripheral);
    }
  });
}

Device::~Device() {
  logger.Log(kLoggerLevel_Info, "Attempting to clean up communication manager...");
  communication_manager_.reset();

  logger.Log(kLoggerLevel_Info, "Successfully cleaned up communication manager");
}