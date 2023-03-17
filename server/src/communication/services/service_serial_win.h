// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <Windows.h>

#include <atomic>
#include <string>

#include "communication/services/communication_service.h"
#include "opengloves_interface.h"

class SerialCommunicationService : public ICommunicationService {
 public:
  explicit SerialCommunicationService(og::DeviceSerialCommunicationConfiguration configuration);

  bool ReceiveNextPacket(std::string& buff) override;
  bool RawWrite(const std::string& buff) override;

  bool IsConnected() override;

  bool PrepareDisconnect() override;

  std::string GetIdentifier() override;

  ~SerialCommunicationService();

 private:
  bool CancelIO();

  bool Connect();
  bool Disconnect();

  void LogError(const std::string&, bool with_win_error);

  og::DeviceSerialCommunicationConfiguration configuration_;

  HANDLE handle_;

  std::atomic<bool> is_connected_ = false;
  std::atomic<bool> is_disconnecting_ = false;
};