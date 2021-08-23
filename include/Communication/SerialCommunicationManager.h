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

static const char* c_serialCommunicationSettingsSection = "communication_serial";

class SerialCommunicationManager : public CommunicationManager {
 public:
  SerialCommunicationManager(std::unique_ptr<IEncodingManager> encodingManager, const VRSerialConfiguration_t& configuration);

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
  bool PurgeBuffer();

 private:
  VRSerialConfiguration_t m_serialConfiguration;

  std::atomic<bool> m_isConnected;

  std::atomic<HANDLE> m_hSerial;
};