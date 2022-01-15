#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

struct DeviceConnectionEventData {
  DeviceConnectionEventData(bool connected) : connected(connected), valid(true){};
  bool valid = false;
  bool connected = false;
};

typedef union CommunicationStateEventData {
  CommunicationStateEventData(){};
  DeviceConnectionEventData deviceConnectionEventData;
} VRStateEventData_t;

enum class CommunicationStateEventType : int { DeviceConnectionEvent };

struct CommunicationStateEvent {
  CommunicationStateEvent(CommunicationStateEventType type, CommunicationStateEventData data) : type(type), data(data){};
  CommunicationStateEventType type;
  VRStateEventData_t data;
};

class CommunicationManager {
 public:
  explicit CommunicationManager(const VRDeviceConfiguration& deviceConfiguration);
  CommunicationManager(std::unique_ptr<EncodingManager> encodingManager, const VRDeviceConfiguration& deviceConfiguration);

  virtual void BeginListener(
      const std::function<void(VRInputData)>& onInputUpdateCallback, const std::function<void(CommunicationStateEvent)>& onStateUpdateCallback);

  virtual void Disconnect();
  virtual void DisconnectAndReconnect();

  virtual void QueueSend(const VRFFBData& data);

  virtual bool IsConnected() = 0;

 protected:
  virtual void SendConnectionStateUpdate(bool connected);
  virtual void ListenerThread();
  virtual void WaitAttemptConnection();

  virtual bool Connect() = 0;
  virtual bool DisconnectFromDevice() = 0;
  virtual void LogError(const char* message) = 0;
  virtual void LogMessage(const char* message) = 0;
  virtual bool ReceiveNextPacket(std::string& buff) = 0;
  virtual bool SendMessageToDevice() = 0;

  std::unique_ptr<EncodingManager> encodingManager_;
  VRDeviceConfiguration deviceConfiguration_;

  std::function<void(VRInputData)> onInputUpdateCallback_;
  std::function<void(CommunicationStateEvent)> onStateUpdateCallback_;

  std::atomic<bool> threadActive_;
  std::thread thread_;

  std::mutex writeMutex_;
  std::string writeString_;
};