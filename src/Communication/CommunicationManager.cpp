#include "Communication/CommunicationManager.h"

#include <chrono>

#include "DeviceConfiguration.h"

static const uint32_t c_listenerWaitTime = 1000;

CommunicationManager::CommunicationManager(const VRDeviceConfiguration& deviceConfiguration) : CommunicationManager(nullptr, deviceConfiguration) {}

CommunicationManager::CommunicationManager(std::unique_ptr<EncodingManager> encodingManager, const VRDeviceConfiguration& deviceConfiguration)
    : encodingManager_(std::move(encodingManager)), deviceConfiguration_(deviceConfiguration), threadActive_(false) {
  // initially no force feedback
  QueueSend(VRFFBData(0, 0, 0, 0, 0));
}

void CommunicationManager::BeginListener(
    const std::function<void(VRInputData)>& onInputUpdateCallback, const std::function<void(CommunicationStateEvent)>& onStateUpdateCallback) {
  onInputUpdateCallback_ = onInputUpdateCallback;
  onStateUpdateCallback_ = onStateUpdateCallback;

  threadActive_ = true;
  thread_ = std::thread(&CommunicationManager::ListenerThread, this);
}

void CommunicationManager::DisconnectAndReconnect() {
  if (DisconnectFromDevice()) {
    SendConnectionStateUpdate(false);
    LogMessage("Disconnected successfully. Attempting to reconnect...");
    WaitAttemptConnection();
  } else {
    LogMessage("Unable to disconnect from device, fully closing listener.");
    threadActive_ = false;
  }
}

void CommunicationManager::Disconnect() {
  if (threadActive_.exchange(false)) thread_.join();

  if (IsConnected()) {
    if (DisconnectFromDevice()) {
      SendConnectionStateUpdate(false);
      LogMessage("Successfully disconnected from device.");
    } else {
      LogError("Unable to disconnect from device.");
    }
  } else
    LogMessage("Did not disconnect as already disconnected.");
}

void CommunicationManager::QueueSend(const VRFFBData& data) {
  std::lock_guard lock(writeMutex_);

  if (encodingManager_ != nullptr) writeString_ = encodingManager_->Encode(data);
}

void CommunicationManager::ListenerThread() {
  WaitAttemptConnection();

  while (threadActive_) {
    std::string receivedString;
    if (ReceiveNextPacket(receivedString)) {
      try {
        const VRInputData commData = encodingManager_->Decode(receivedString);
        onInputUpdateCallback_(commData);

        if (deviceConfiguration_.feedbackEnabled) {
          SendMessageToDevice();
        }

        continue;
      } catch (const std::invalid_argument& ia) {
        LogMessage((std::string("Received error from encoding manager: ") + ia.what()).c_str());
      }
    }

    LogMessage("Detected device error. Disconnecting socket and attempting reconnection...");

    DisconnectAndReconnect();
  }
}

void CommunicationManager::SendConnectionStateUpdate(bool connected) {
  CommunicationStateEventData stateData;
  stateData.deviceConnectionEventData = DeviceConnectionEventData(connected);

  onStateUpdateCallback_({CommunicationStateEventType::DeviceConnectionEvent, stateData});
}

void CommunicationManager::WaitAttemptConnection() {
  while (threadActive_ && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }

  SendConnectionStateUpdate(true);
}
