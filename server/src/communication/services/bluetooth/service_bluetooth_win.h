#pragma once

#include <WinSock2.h>
#include <bluetoothapis.h>


#include "communication_service.h"

class BluetoothCommunicationService : public ICommunicationService {
 public:
  explicit BluetoothCommunicationService(BTH_ADDR bt_address);

  int Connect() override {
    return 0;
  }

  void AttachEventHandler(std::function<void(CommunicationServiceEvent event)> callback){};

  int ReceiveNextPacket(std::string& buff) {
    return 0;
  };

  std::string GetAddress() {
    return "";
  }

  int Disconnect() {
    return 0;
  }

 private:
  BTH_ADDR bt_address_;
};