#pragma once

#include <WinSock2.h>
#include <bluetoothapis.h>

#include <atomic>

#include "services/communication_service.h"

class BluetoothCommunicationService : public ICommunicationService {
 public:
  explicit BluetoothCommunicationService(BTH_ADDR bt_address);

  bool ReceiveNextPacket(std::string& buff) override;
  bool RawWrite(const std::string& buff) override;

  bool IsConnected() override;

  bool PurgeBuffer() override;

  ~BluetoothCommunicationService();

 private:
  bool Connect();
  bool Disconnect();

  void LogError(const std::string&, bool with_win_error);

  BTH_ADDR bt_address_;

  SOCKET sock_;

  std::atomic<bool> is_connected_;
  std::atomic<bool> is_disconnecting_;
};