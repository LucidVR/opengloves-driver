#pragma once

#include <string>

#include "../../../../include/communication/services/communication_service.h"

class SerialCommunicationService : public CommunicationService {
 public:
  SerialCommunicationService(const std::string& port_name);

  int ReceiveNextPacket(std::string& buff) override;

  int Connect() override;

  void AttachEventHandler(std::function<void(CommunicationServiceEvent event)> callback) override;

 private:
  std::string port_name;
};