#pragma once

#include <Windows.h>

#include <atomic>
#include <string>

#include "services/communication_service.h"

class SerialCommunicationService : public ICommunicationService {
 public:
  explicit SerialCommunicationService(const std::string& port_name);

  bool ReceiveNextPacket(std::string& buff) override;
  bool RawWrite(const std::string& buff) override;

  bool IsConnected() override;

  bool PurgeBuffer() override;

  ~SerialCommunicationService();

 private:
  bool CancelIO();

  bool Connect();
  bool Disconnect();

  void LogError(const std::string&, bool with_win_error);

  std::string port_name_;

  HANDLE handle_;

  std::atomic<bool> is_connected_ = false;
  std::atomic<bool> is_disconnecting_ = false;
};