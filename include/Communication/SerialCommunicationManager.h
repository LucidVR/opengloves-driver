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
  SerialCommunicationManager(
      std::unique_ptr<EncodingManager> encodingManager,
      const VRSerialConfiguration& configuration,
      const VRDeviceConfiguration& deviceConfiguration);

  bool IsConnected() override;

 protected:
  bool Connect() override;
  bool DisconnectFromDevice() override;
  void LogError(const char* message) override;
  void LogMessage(const char* message) override;
  bool ReceiveNextPacket(std::string& buff) override;
  bool SendMessageToDevice() override;

 private:
  bool PurgeBuffer();

  VRSerialConfiguration m_serialConfiguration;
  std::atomic<bool> m_isConnected;
  std::atomic<HANDLE> m_hSerial;
};