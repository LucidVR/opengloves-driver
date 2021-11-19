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

void CommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  threadActive_ = true;
  thread_ = std::thread(&CommunicationManager::ListenerThread, this, callback);
}

void CommunicationManager::Disconnect() {
  if (threadActive_.exchange(false)) thread_.join();

  if (IsConnected()) DisconnectFromDevice();
}

void CommunicationManager::QueueSend(const VRFFBData& data) {
  std::lock_guard lock(writeMutex_);

  if(encodingManager_ != nullptr) writeString_ = encodingManager_->Encode(data);
}

void CommunicationManager::ListenerThread(const std::function<void(VRInputData)>& callback) {
  WaitAttemptConnection();

  while (threadActive_) {
    if (std::string receivedString; ReceiveNextPacket(receivedString)) {
      try {
        const VRInputData commData = encodingManager_->Decode(receivedString);
        callback(commData);

        if (deviceConfiguration_.feedbackEnabled) {
          SendMessageToDevice();
        }

        continue;
      } catch (const std::invalid_argument& ia) {
        LogMessage((std::string("Received error from encoding manager: ") + ia.what()).c_str());
      }
    }

    LogMessage("Detected device error. Disconnecting socket and attempting reconnection...");

    if (DisconnectFromDevice()) {
      WaitAttemptConnection();
      LogMessage("Successfully reconnected to device.");
    } else {
      LogMessage("Could not disconnect. Closing listener...");
      Disconnect();
    }
  }
}

void CommunicationManager::WaitAttemptConnection() {
  while (threadActive_ && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }
}
