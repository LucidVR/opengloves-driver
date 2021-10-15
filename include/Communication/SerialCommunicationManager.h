#pragma once

#include <Windows.h>

#include <atomic>
#include <memory>
#include <string>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

class SerialCommunicationManager : public CommunicationManager {
 public:
  SerialCommunicationManager(std::unique_ptr<EncodingManager> encodingManager, VRSerialConfiguration_t configuration, const VRDeviceConfiguration_t& deviceConfiguration);

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