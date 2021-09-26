#pragma once

#include <memory>
#include <atomic>
#include "Util/NamedPipeListener.h"

#include "DeviceConfiguration.h"
#include "Communication/CommunicationManager.h"

class NamedPipeCommunicationManager : public CommunicationManager {

 public:
  NamedPipeCommunicationManager(const VRNamedPipeInputConfiguration& configuration);
  bool IsConnected();

 protected:
  bool Connect();
  bool DisconnectFromDevice();
  void LogError(const char* message);
  void LogMessage(const char* message);

  //Functions currently unimplemeneted
  bool SendMessageToDevice() { return true; };
  bool ReceiveNextPacket(std::string& buff) { return true; };

  void BeginListener(const std::function<void(VRInputData)>& callback) override;

 private:
  std::unique_ptr<NamedPipeListener<VRInputData>> m_namedPipeListener;
  std::atomic<bool> m_isConnected;
  VRNamedPipeInputConfiguration m_configuration;
};