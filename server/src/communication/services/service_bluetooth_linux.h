#pragma once

#include "communication_service.h"
#include "opengloves_interface.h"

#include <atomic>

class BluetoothCommunicationService : public ICommunicationService {
 public:
  explicit BluetoothCommunicationService(og::DeviceBluetoothCommunicationConfiguration configuration);

  bool ReceiveNextPacket(std::string& buff) override;
  bool RawWrite(const std::string& buff) override;

  bool IsConnected() override;

  bool PurgeBuffer() override;

  std::string GetIdentifier() override;

  ~BluetoothCommunicationService() override;

 private:

  void Disconnect();

  og::DeviceBluetoothCommunicationConfiguration configuration_;

  std::atomic<int> socket_;

  std::atomic<bool> is_connected_;
};