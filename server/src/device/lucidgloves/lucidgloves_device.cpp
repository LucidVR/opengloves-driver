// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "lucidgloves_device.h"

#include "opengloves_interface.h"
#include "services/input/input_force_feedback_named_pipe.h"
#include "services/output/output_osc.h"

using namespace og;

class LucidglovesDevice::Impl {
 public:
  Impl(og::Hand hand, std::unique_ptr<ICommunicationManager> communication_manager)
      : hand_(hand), communication_manager_(std::move(communication_manager)) {
    force_feedback_ = std::make_unique<InputForceFeedbackNamedPipe>(hand_, [&](const ForceFeedbackCurlData &curl_data) {

      const og::Output output = {
          .type = og::kOutputData_Type_ForceFeedback,
          .data = {
              .force_feedback_data = {
                  .thumb = curl_data.thumb,
                  .index = curl_data.index,
                  .middle = curl_data.middle,
                  .ring = curl_data.ring,
                  .pinky = curl_data.pinky
              }
          },
      };

      communication_manager_->WriteOutput(output);
    });
  };

  void ListenForInput(std::function<void(const og::InputPeripheralData &)> callback) {
    callback_ = std::move(callback);

    communication_manager_->BeginListener([&](const Input &data) {
      if (data.type == kInputDataType_Peripheral) {
        callback_(data.data.peripheral);

        OutputOSCServer::GetInstance().Send(hand_, data.data.peripheral);
      }
    });

    force_feedback_->StartListener();
  }

  void Output(const og::Output &output) {
    communication_manager_->WriteOutput(output);
  }

  ~Impl() {
    force_feedback_ = nullptr;
    communication_manager_ = nullptr;
  }

 private:
  og::Hand hand_;

  std::function<void(og::InputPeripheralData)> callback_;
  std::unique_ptr<ICommunicationManager> communication_manager_;
  std::unique_ptr<InputForceFeedbackNamedPipe> force_feedback_;
};

LucidglovesDevice::LucidglovesDevice(og::DeviceConfiguration configuration, std::unique_ptr<ICommunicationManager> communication_manager)
    : configuration_(std::move(configuration)) {
  pImpl_ = std::make_unique<Impl>(configuration_.hand, std::move(communication_manager));

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