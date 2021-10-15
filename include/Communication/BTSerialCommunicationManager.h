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
  BTSerialCommunicationManager(std::unique_ptr<EncodingManager> encodingManager, VRBTSerialConfiguration_t configuration,
                               const VRDeviceConfiguration_t& deviceConfiguration);

 public:
  bool IsConnected();

 protected:
  bool Connect();
  bool DisconnectFromDevice();
  void LogError(const char* message);
  void LogMessage(const char* message);
  bool ReceiveNextPacket(std::string& buff);
  bool SendMessageToDevice();

 private:
  bool ConnectToDevice(BTH_ADDR& deviceBtAddress);
  bool GetPairedDeviceBtAddress(BTH_ADDR* deviceBtAddress);
  bool StartupWindowsSocket();

 private:
  VRBTSerialConfiguration_t m_btSerialConfiguration;

  std::atomic<bool> m_isConnected;

  std::atomic<SOCKET> m_btClientSocket;
};