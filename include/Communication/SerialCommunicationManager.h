#pragma once

#include <windows.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DriverLog.h"
#include "Util/Util.h"

class SerialCommunicationManager : public ICommunicationManager {
 public:
  SerialCommunicationManager(std::unique_ptr<IEncodingManager> encodingManager, const VRSerialConfiguration_t& configuration);

#pragma region ICommunicationManager
 public:
  bool IsConnected();

 protected:
  bool Connect();
  bool DisconnectFromDevice();
  void LogError(const char* message);
  void LogMessage(const char* message);
  bool ReceiveNextPacket(std::string& buff);
  bool SendMessageToDevice();
#pragma endregion

#pragma region Core logic
 private:
  bool PurgeBuffer();
#pragma endregion

 private:
  VRSerialConfiguration_t m_serialConfiguration;

  std::atomic<bool> m_isConnected;

  std::atomic<HANDLE> m_hSerial;
};