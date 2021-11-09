#pragma once

#include <atomic>
#include <memory>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "Util/NamedPipeListener.h"

class NamedPipeCommunicationManager : public CommunicationManager {
 public:
  NamedPipeCommunicationManager(VRNamedPipeInputConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration);
  bool IsConnected() override;

 protected:
  bool Connect() override;
  bool DisconnectFromDevice() override;
  void LogError(const char* message) override;
  void LogMessage(const char* message) override;

  // Functions currently unimplemented
  bool SendMessageToDevice() override {
    return true;
  };
  bool ReceiveNextPacket(std::string& buff) override {
    return true;
  };
  void QueueSend(const VRFFBData& data) override{};

  void BeginListener(const std::function<void(VRInputData)>& callback) override;

 private:
  std::unique_ptr<NamedPipeListener<VRInputData>> _namedPipeListener;
  std::atomic<bool> _isConnected;

  std::function<void(VRInputData)> _callback;

  VRNamedPipeInputConfiguration _configuration;
};