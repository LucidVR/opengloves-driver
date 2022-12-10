#pragma once

#pragma once

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

  bool PurgeBuffer() override;

  std::string GetIdentifier() override;

  ~SerialCommunicationService();

 private:
  og::DeviceSerialCommunicationConfiguration configuration_;

  std::atomic<int> fd_;

  std::atomic<bool> is_connected_;
};