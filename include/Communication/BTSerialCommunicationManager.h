#pragma once
#include <Winsock2.h>
#include <Ws2bth.h>
#include <bluetoothapis.h>
#include <windows.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DriverLog.h"
#include "Util/Util.h"

static const char* c_btserialCommunicationSettingsSection = "communication_btserial";

class BTSerialCommunicationManager : public CommunicationManager {
 public:
  BTSerialCommunicationManager(std::unique_ptr<IEncodingManager> encodingManager, const VRBTSerialConfiguration_t& configuration);

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