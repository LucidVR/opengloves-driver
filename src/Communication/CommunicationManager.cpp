#include "Communication/CommunicationManager.h"

#include <chrono>

static const uint32_t c_listenerWaitTime = 1000;

CommunicationManager::CommunicationManager(std::unique_ptr<EncodingManager> encodingManager)
    : m_encodingManager(std::move(encodingManager)), m_threadActive(false), m_writeString() {
  // initially no force feedback
  QueueSend(VRFFBData(0, 0, 0, 0, 0));
}

void CommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  m_threadActive = true;
  m_thread = std::thread(&CommunicationManager::ListenerThread, this, callback);
}

void CommunicationManager::Disconnect() {
  if (m_threadActive.exchange(false)) m_thread.join();

  if (IsConnected()) DisconnectFromDevice();
}

void CommunicationManager::QueueSend(const VRFFBData& data) {
  std::lock_guard<std::mutex> lock(m_writeMutex);

  m_writeString = m_encodingManager->Encode(data);
}

void CommunicationManager::ListenerThread(const std::function<void(VRInputData)>& callback) {
  WaitAttemptConnection();

  while (m_threadActive) {
    std::string receivedString;
    bool readSuccessful = ReceiveNextPacket(receivedString);

    if (readSuccessful) {
      try {
        VRInputData commData = m_encodingManager->Decode(receivedString);
        callback(commData);
        SendMessageToDevice();
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
  while (m_threadActive && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }
}
