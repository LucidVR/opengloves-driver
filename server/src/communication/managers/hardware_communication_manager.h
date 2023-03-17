// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>

#include "communication/encoding/encoding_service.h"
#include "communication/services/communication_service.h"
#include "communication_manager.h"

/**
 * A communication manager that manages a resources that can be queried and writes to/from the device in strings using an encoding scheme. Ie.
 * Bluetooth, Serial, etc. But **NOT** ipc methods like named pipes.
 */
class HardwareCommunicationManager : public ICommunicationManager {
 public:
  HardwareCommunicationManager(std::unique_ptr<ICommunicationService> communication_service, std::unique_ptr<IEncodingService> encoding_service);

  void BeginListener(std::function<void(const og::Input&)> callback) override;

  void WriteOutput(const og::Output& output) override;

  ~HardwareCommunicationManager() override;

 private:
  void CommunicationThread();

  std::atomic<bool> thread_active_;
  std::thread communication_thread_;

  std::string queued_write_string;

  std::function<void(const og::Input&)> callback_;

  std::unique_ptr<ICommunicationService> communication_service_;
  std::unique_ptr<IEncodingService> encoding_service_;
};