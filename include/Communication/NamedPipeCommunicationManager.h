#pragma once

#include <array>
#include <atomic>
#include <memory>

#include "Communication/CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "Util/NamedPipeListener.h"

class NamedPipeCommunicationManager : public CommunicationManager {
 public:
  NamedPipeCommunicationManager(const VRCommunicationConfiguration& configuration);
  bool IsConnected() override;

  // no sending for named pipes
  void QueueSend(const VROutput& data) override{};
  void BeginListener(const std::function<void(VRInputData)>& callback) override;

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

 private:
  std::atomic<bool> isConnected_;

  std::function<void(VRInputData)> callback_;

  VRCommunicationNamedPipeConfiguration namedPipeConfiguration_;

  std::vector<std::unique_ptr<IListener>> namedPipeListeners_;
};