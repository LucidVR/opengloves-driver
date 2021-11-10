#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

class CommunicationManager {
 public:
  explicit CommunicationManager(const VRDeviceConfiguration& deviceConfiguration);
  CommunicationManager(std::unique_ptr<EncodingManager> encodingManager, const VRDeviceConfiguration& deviceConfiguration);

  virtual void BeginListener(const std::function<void(VRInputData)>& callback);
  virtual void Disconnect();
  virtual void QueueSend(const VRFFBData& data);

  virtual bool IsConnected() = 0;

 protected:
  virtual void ListenerThread(const std::function<void(VRInputData)>& callback);
  virtual void WaitAttemptConnection();

  virtual bool Connect() = 0;
  virtual bool DisconnectFromDevice() = 0;
  virtual void LogError(const char* message) = 0;
  virtual void LogMessage(const char* message) = 0;
  virtual bool ReceiveNextPacket(std::string& buff) = 0;
  virtual bool SendMessageToDevice() = 0;

  std::unique_ptr<EncodingManager> encodingManager_;
  VRDeviceConfiguration deviceConfiguration_;

  std::atomic<bool> threadActive_;
  std::thread thread_;

  std::mutex writeMutex_;
  std::string writeString_;
};