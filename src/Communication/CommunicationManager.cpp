#include <Communication/CommunicationManager.h>

static const uint32_t c_listenerWaitTime = 1000;

ICommunicationManager::ICommunicationManager(std::unique_ptr<IEncodingManager> encodingManager)
    : m_encodingManager(std::move(encodingManager)), m_threadActive(false), m_writeString() {
  // initially no force feedback
  QueueSend(VRFFBData_t(0, 0, 0, 0, 0));
}

#pragma region Public

void ICommunicationManager::BeginListener(const std::function<void(VRCommData_t)>& callback) {
  m_threadActive = true;
  m_thread = std::thread(&CommunicationManager::ListenerThread, this, callback);
}

void ICommunicationManager::Disconnect() {
  if (IsConnected()) {
    if (m_threadActive.exchange(false)) m_serialThread.join();
    DisconnectFromDevice();
  }
}

void ICommunicationManager::QueueSend(const VRFFBData_t& data) {
  std::lock_guard<std::mutex> lock(m_writeMutex);

  m_writeString = m_encodingManager->Encode(data);
}

#pragma endregion

#pragma region Protected

void ICommunicationManager::ListenerThread(const std::function<void(VRCommData_t)>& callback) {
  WaitAttemptConnection();

  while (m_threadActive) {
    std::string receivedString;
    bool readSuccessful = ReceiveNextPacket(receivedString);

    if (readSuccessful) {
      try {
        VRCommData_t commData = m_encodingManager->Decode(receivedString);
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

void ICommunicationManager::WaitAttemptConnection() {
  while (m_threadActive && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }
}

#pragma endregion
