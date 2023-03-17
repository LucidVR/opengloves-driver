// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "hardware_communication_manager.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

HardwareCommunicationManager::HardwareCommunicationManager(
    std::unique_ptr<ICommunicationService> communication_service, std::unique_ptr<IEncodingService> encoding_service) {
  communication_service_ = std::move(communication_service);
  encoding_service_ = std::move(encoding_service);
}

void HardwareCommunicationManager::BeginListener(std::function<void(const og::Input&)> callback) {
  if (thread_active_) {
    logger.Log(kLoggerLevel_Warning, "Did not start communication listener as the listener was already active.");
    return;
  }

  callback_ = callback;

  thread_active_ = true;

  communication_thread_ = std::thread(&HardwareCommunicationManager::CommunicationThread, this);
}

void HardwareCommunicationManager::CommunicationThread() {
  while (thread_active_) {
    std::string received_string;
    if (!communication_service_->ReceiveNextPacket(received_string)) {
      logger.Log(kLoggerLevel_Error, "Failed to read from device.");

      return;
    }

    const Input input = encoding_service_->DecodePacket(received_string);
    callback_(input);

    // now write information we might have
    queued_write_string += "\n";
    if (!communication_service_->RawWrite(queued_write_string)) {
      logger.Log(kLoggerLevel_Error, "Failed to write to device.");

      return;
    }
    if (queued_write_string != "\n")  // log any data we've sent to the device
      logger.Log(kLoggerLevel_Info, "Wrote data to device: %s", queued_write_string.c_str());

    queued_write_string.clear();
  }
}

void HardwareCommunicationManager::WriteOutput(const og::Output& output) {
  const std::string encoded_string = encoding_service_->EncodePacket(output);
  queued_write_string += encoded_string;
}

HardwareCommunicationManager::~HardwareCommunicationManager() {
  if (thread_active_.exchange(false)) {
    communication_service_->PrepareDisconnect();

    logger.Log(kLoggerLevel_Info, "Attempting to cleanup communication thread...");
    communication_thread_.join();

    logger.Log(kLoggerLevel_Info, "Successfully cleaned up communication thread");
  }
}