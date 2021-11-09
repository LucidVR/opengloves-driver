#include "Communication/CommunicationManager.h"

#include <chrono>

#include "DeviceConfiguration.h"

static const uint32_t c_listenerWaitTime = 1000;

CommunicationManager::CommunicationManager(const VRDeviceConfiguration& deviceConfiguration) : CommunicationManager(nullptr, deviceConfiguration) {}

CommunicationManager::CommunicationManager(std::unique_ptr<EncodingManager> encodingManager, const VRDeviceConfiguration& deviceConfiguration)
    : _encodingManager(std::move(encodingManager)), _deviceConfiguration(deviceConfiguration), _threadActive(false) {
  // initially no force feedback
  QueueSend(VRFFBData(0, 0, 0, 0, 0));
}

void CommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  _threadActive = true;
  _thread = std::thread(&CommunicationManager::ListenerThread, this, callback);
}

void CommunicationManager::Disconnect() {
  if (_threadActive.exchange(false)) _thread.join();

  if (IsConnected()) DisconnectFromDevice();
}

void CommunicationManager::QueueSend(const VRFFBData& data) {
  std::lock_guard lock(_writeMutex);

  _writeString = _encodingManager->Encode(data);
}

void CommunicationManager::ListenerThread(const std::function<void(VRInputData)>& callback) {
  WaitAttemptConnection();

  while (_threadActive) {
    if (std::string receivedString; ReceiveNextPacket(receivedString)) {
      try {
        const VRInputData commData = _encodingManager->Decode(receivedString);
        callback(commData);

        if (_deviceConfiguration.feedbackEnabled) {
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
  while (_threadActive && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }
}
