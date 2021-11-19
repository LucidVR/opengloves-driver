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
  void QueueSend(const VRFFBData& data) override{};

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

  void BeginListener(const std::function<void(VRInputData)>& callback) override;

 private:
  std::unique_ptr<NamedPipeListener<VRInputData>> namedPipeListener_;
  std::atomic<bool> isConnected_;

  std::function<void(VRInputData)> callback_;

  VRNamedPipeInputConfiguration configuration_;
};