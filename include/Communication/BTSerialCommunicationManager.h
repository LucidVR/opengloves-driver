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

class BTSerialCommunicationManager : public ICommunicationManager {
 public:
  BTSerialCommunicationManager(std::unique_ptr<IEncodingManager> encodingManager, const VRBTSerialConfiguration_t& configuration);

#pragma region ICommunicationManager
 public:
  bool IsConnected();

 protected:
  bool Connect();
  bool DisconnectFromDevice();
  void LogError(const char* message);
  void LogMessage(const char* message);
  bool ReceiveNextPacket(std::string& buff);
#pragma endregion

#pragma region Core logic
 private:
  bool ConnectToDevice(BTH_ADDR& deviceBtAddress);
  bool GetPairedDeviceBtAddress(BTH_ADDR* deviceBtAddress);
  bool SendMessageToDevice();
  bool StartupWindowsSocket();
#pragma endregion

 private:
  VRBTSerialConfiguration_t m_btSerialConfiguration;

  std::atomic<bool> m_isConnected;

  std::atomic<SOCKET> m_btClientSocket;
};