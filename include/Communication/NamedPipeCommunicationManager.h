#pragma once

#include <atomic>
#include <memory>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "Util/NamedPipeListener.h"

class NamedPipeCommunicationManager : public CommunicationManager {
 public:
  NamedPipeCommunicationManager(
      const VRNamedPipeInputConfiguration& configuration, const VRDeviceConfiguration& deviceConfiguration);
  bool IsConnected();

 protected:
  bool Connect();
  bool DisconnectFromDevice();
  void LogError(const char* message);
  void LogMessage(const char* message);

  // Functions currently unimplemeneted
  bool SendMessageToDevice() {
    return true;
  };
  bool ReceiveNextPacket(std::string& buff) {
    return true;
  };
  void QueueSend(const VRFFBData& data) override{};

  void BeginListener(const std::function<void(VRInputData)>& callback) override;

 private:
  std::unique_ptr<NamedPipeListener<VRInputData>> m_namedPipeListener;
  std::atomic<bool> m_isConnected;

  std::function<void(VRInputData)> m_callback;

  VRNamedPipeInputConfiguration m_configuration;
};