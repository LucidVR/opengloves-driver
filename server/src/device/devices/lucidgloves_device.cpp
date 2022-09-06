#include "devices/lucidgloves_device.h"

#include "services/input/input_force_feedback_named_pipe.h"
#include "services/output/output_osc.h"

using namespace og;

class LucidglovesDevice::Impl {
 public:
  explicit Impl(og::Hand hand) : hand_(hand){};

  void ListenForInput(std::function<void(const og::InputPeripheralData &)> callback) {
    callback_ = std::move(callback);

    communication_manager_->BeginListener([&](Input data) {
      if (data.type == kInputDataType_Peripheral) {
        callback_(data.data.peripheral);

        OutputOSCServer::GetInstance().Send(hand_, data.data.peripheral);
      }
    });
  }

 private:
  og::Hand hand_;

  std::function<void(og::InputPeripheralData)> callback_;
  std::unique_ptr<ICommunicationManager> communication_manager_;
  std::unique_ptr<OutputForceFeedbackData> force_feedback_;
};

LucidglovesDevice::LucidglovesDevice(og::DeviceConfiguration configuration, std::unique_ptr<ICommunicationManager> communication_manager)
    : configuration_(std::move(configuration)), pImpl_(std::make_unique<Impl>(configuration_.hand)) {
  OutputOSCServer::GetInstance();
};

void LucidglovesDevice::ListenForInput(std::function<void(const og::InputPeripheralData &)> callback) {}

og::DeviceConfiguration LucidglovesDevice::GetConfiguration() {
  return configuration_;
}

LucidglovesDevice::~LucidglovesDevice() {
  OutputOSCServer::GetInstance().Stop();
}