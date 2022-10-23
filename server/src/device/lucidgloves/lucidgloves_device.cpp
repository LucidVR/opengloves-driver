#include "lucidgloves_device.h"

#include "services/input/input_force_feedback_named_pipe.h"
#include "services/output/output_osc.h"

using namespace og;

class LucidglovesDevice::Impl {
 public:
  Impl(og::Hand hand, std::unique_ptr<ICommunicationManager> communication_manager)
      : hand_(hand),
        communication_manager_(std::move(communication_manager)),
        force_feedback_(std::make_unique<InputForceFeedbackNamedPipe>(hand, [&](const ForceFeedbackCurlData &curl_data) {
          og::Output output{};

          og::OutputForceFeedbackData output_force_feedback_data{};
          output_force_feedback_data.thumb = curl_data.thumb;
          output_force_feedback_data.index = curl_data.index;
          output_force_feedback_data.middle = curl_data.middle;
          output_force_feedback_data.ring = curl_data.ring;
          output_force_feedback_data.pinky = curl_data.pinky;

          output.data.force_feedback_data = output_force_feedback_data;

          output.type = og::kOutputData_Type_ForceFeedback;

          communication_manager_->WriteOutput(output);
        })){};

  void ListenForInput(std::function<void(const og::InputPeripheralData &)> callback) {
    callback_ = std::move(callback);

    communication_manager_->BeginListener([&](const Input &data) {
      if (data.type == kInputDataType_Peripheral) {
        callback_(data.data.peripheral);

        OutputOSCServer::GetInstance().Send(hand_, data.data.peripheral);
      }
    });
  }

  void Output(const og::Output &output) {
    communication_manager_->WriteOutput(output);
  }

 private:
  og::Hand hand_;

  std::function<void(og::InputPeripheralData)> callback_;
  std::unique_ptr<ICommunicationManager> communication_manager_;
  std::unique_ptr<InputForceFeedbackNamedPipe> force_feedback_;
};

LucidglovesDevice::LucidglovesDevice(og::DeviceConfiguration configuration, std::unique_ptr<ICommunicationManager> communication_manager)
    : configuration_(std::move(configuration)), pImpl_(std::make_unique<Impl>(configuration_.hand, std::move(communication_manager))) {
  OutputOSCServer::GetInstance();
};

void LucidglovesDevice::ListenForInput(std::function<void(const og::InputPeripheralData &)> callback) {
  pImpl_->ListenForInput(callback);
}

void LucidglovesDevice::Output(const og::Output &output) {
  pImpl_->Output(output);
}

og::DeviceConfiguration LucidglovesDevice::GetConfiguration() {
  return configuration_;
}

LucidglovesDevice::~LucidglovesDevice() {
  pImpl_ = nullptr;
}