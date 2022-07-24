#pragma once

#include <string>

#include "services/communication_service.h"
#include <windows.h>

class SerialCommunicationService : public ICommunicationService {
 public:
  explicit SerialCommunicationService(const std::string& port_name);

  int ReceiveNextPacket(std::string& buff) override;

  int Connect() override;

  void AttachEventHandler(std::function<void(CommunicationServiceEvent event)> callback) override;

 private:
  std::string port_name_;

  HANDLE handle_;
  bool is_connected_;
};