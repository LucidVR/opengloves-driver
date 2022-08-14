#include "Communication/CommunicationManager.h"

#include <chrono>

#include "DeviceConfiguration.h"

static const uint32_t c_listenerWaitTime = 1000;

CommunicationManager::CommunicationManager(VRCommunicationConfiguration configuration) : CommunicationManager(std::move(configuration), nullptr) {}

CommunicationManager::CommunicationManager(VRCommunicationConfiguration configuration, std::unique_ptr<EncodingManager> encodingManager)
    : encodingManager_(std::move(encodingManager)), configuration_(std::move(configuration)), threadActive_(false) {
  // initially no force feedback
  QueueSend(VRFFBData(0, 0, 0, 0, 0));
}

void CommunicationManager::BeginListener(const std::function<void(const VRInputData&)>& callback) {
  threadActive_ = true;
  thread_ = std::thread(&CommunicationManager::ListenerThread, this, callback);
}

void CommunicationManager::Disconnect() {
  if (threadActive_.exchange(false)) {
    // do anything needed to get ready to disconnect (cancelling read/write operations)
    PrepareDisconnection();

    // now wait for the thread to join
    thread_.join();

    // then disconnect fully
    DisconnectFromDevice();
  }
}

void CommunicationManager::QueueSend(const VROutput& data) {
  std::lock_guard lock(writeMutex_);

  if (encodingManager_ != nullptr && configuration_.feedbackEnabled) writeString_ += encodingManager_->Encode(data);
}

void CommunicationManager::ListenerThread(const std::function<void(VRInputData)>& callback) {
  WaitAttemptConnection();

  while (threadActive_) {
    if (std::string receivedString; ReceiveNextPacket(receivedString)) {
      try {
        const VRInputData commData = encodingManager_->Decode(receivedString);
        callback(commData);

        if (configuration_.feedbackEnabled) {
          std::lock_guard lock(writeMutex_);

          // append a newline and send
          writeString_ = writeString_ + "\n";

          SendMessageToDevice();

          if (writeString_ != "\n") DriverLog("Wrote to device: %s", writeString_.c_str());

          writeString_.clear();
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
  LogMessage("Attempting connection to device...");

  while (threadActive_ && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }

  LogMessage("Device successfully connected");

  if (!threadActive_) return;
  // we're now connected

  // discard anything we set beforehand
  writeString_ = "";
}
