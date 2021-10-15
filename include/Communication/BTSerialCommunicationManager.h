#pragma once

#include <WinSock2.h>
#include <bluetoothapis.h>

#include <atomic>
#include <memory>
#include <string>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

class BTSerialCommunicationManager : public CommunicationManager {
 public:
  BTSerialCommunicationManager(
      std::unique_ptr<EncodingManager> encodingManager,
      VRBTSerialConfiguration_t configuration,
      const VRDeviceConfiguration_t& deviceConfiguration);

  bool IsConnected() override;

 protected:
  bool Connect() override;
  bool DisconnectFromDevice() override;
  void LogError(const char* message) override;
  void LogMessage(const char* message) override;
  bool ReceiveNextPacket(std::string& buff) override;
  bool SendMessageToDevice() override;

 private:
  bool ConnectToDevice(BTH_ADDR& deviceBtAddress);
  bool GetPairedDeviceBtAddress(BTH_ADDR* deviceBtAddress);
  bool StartupWindowsSocket();

  VRBTSerialConfiguration_t m_btSerialConfiguration;
  std::atomic<bool> m_isConnected;
  std::atomic<SOCKET> m_btClientSocket;
};
