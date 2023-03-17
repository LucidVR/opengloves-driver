// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "lucidgloves_named_pipe_discovery.h"

#include "communication/managers/named_pipe_communication_manager.h"
#include "device/lucidgloves/lucidgloves_device.h"

static og::Logger& logger = og::Logger::GetInstance();

class LucidglovesNamedPipeDiscovery::Impl {
 public:
  Impl(){};

  void StartListeners(std::function<void(og::Hand hand, std::unique_ptr<ICommunicationManager>)> on_client_connected_callback) {
    on_client_connected_callback_ = std::move(on_client_connected_callback);

    left_named_pipe_manager_ = std::make_unique<NamedPipeCommunicationManager>(og::kHandLeft, [&]() {
      if (left_named_pipe_manager_ == nullptr) return;
      on_client_connected_callback_(og::kHandLeft, std::move(left_named_pipe_manager_));
    });

    right_named_pipe_manager_ = std::make_unique<NamedPipeCommunicationManager>(og::kHandRight, [&]() {
      if (right_named_pipe_manager_ == nullptr) return;
      on_client_connected_callback_(og::kHandRight, std::move(right_named_pipe_manager_));
    });

    left_named_pipe_manager_->BeginListener([&](const og::Input& input) {
    });
    right_named_pipe_manager_->BeginListener([&](const og::Input& input) {
    });
  }

 private:
  std::unique_ptr<NamedPipeCommunicationManager> left_named_pipe_manager_;
  std::unique_ptr<NamedPipeCommunicationManager> right_named_pipe_manager_;

  std::function<void(og::Hand hand, std::unique_ptr<ICommunicationManager>)> on_client_connected_callback_;
  std::function<void(const og::InputData&)> on_data_callback_;
};

LucidglovesNamedPipeDiscovery::LucidglovesNamedPipeDiscovery() {
  pImpl_ = std::make_unique<Impl>();
}

void LucidglovesNamedPipeDiscovery::StartDiscovery(std::function<void(std::unique_ptr<og::IDevice>)> callback) {
  device_discovered_callback_ = std::move(callback);

  logger.Log(og::kLoggerLevel_Info, "Starting named pipe input listener...");

  pImpl_->StartListeners([&](og::Hand hand, std::unique_ptr<ICommunicationManager> communication_manager) {
    og::DeviceConfiguration configuration{};

    configuration.hand = hand;
    configuration.type = og::kDeviceType_lucidgloves;

    device_discovered_callback_(std::make_unique<LucidglovesDevice>(configuration, std::move(communication_manager)));
  });
}

void LucidglovesNamedPipeDiscovery::StopDiscovery() {
  pImpl_.reset();
}

LucidglovesNamedPipeDiscovery::~LucidglovesNamedPipeDiscovery() {
  StopDiscovery();
}