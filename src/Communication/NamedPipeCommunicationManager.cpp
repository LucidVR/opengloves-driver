#include "Communication/NamedPipeCommunicationManager.h"

#include <utility>

NamedPipeCommunicationManager::NamedPipeCommunicationManager(
    VRNamedPipeInputConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration)
    : CommunicationManager(deviceConfiguration), isConnected_(false), configuration_(std::move(configuration)){};

bool NamedPipeCommunicationManager::Connect() {
  namedPipeListener_ = std::make_unique<NamedPipeListener<VRInputData>>(configuration_.pipeName);
  return true;
}

void NamedPipeCommunicationManager::BeginListener(
    const std::function<void(VRInputData)>& onInputUpdateCallback, const std::function<void(CommunicationStateEvent)>& onStateUpdateCallback) {
  onInputUpdateCallback_ = onInputUpdateCallback;
  onStateUpdateCallback_ = onStateUpdateCallback;

  if (!Connect()) {
    DriverLog("Unable to connect to named pipe.");
    return;
  }

  SendConnectionStateUpdate(true);

  namedPipeListener_->StartListening([&](const VRInputData* data) { onInputUpdateCallback_(*data); });
}

bool NamedPipeCommunicationManager::DisconnectFromDevice() {
  namedPipeListener_->StopListening();
  SendConnectionStateUpdate(false);
  return true;
}

bool NamedPipeCommunicationManager::IsConnected() {
  return namedPipeListener_->IsConnected();
}

void NamedPipeCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, configuration_.pipeName.c_str(), GetLastErrorAsString().c_str());
}

void NamedPipeCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, configuration_.pipeName.c_str());
}