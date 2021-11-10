#include "Communication/NamedPipeCommunicationManager.h"

#include <utility>

NamedPipeCommunicationManager::NamedPipeCommunicationManager(
    VRNamedPipeInputConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration)
    : CommunicationManager(deviceConfiguration), isConnected_(false), configuration_(std::move(configuration)){};

bool NamedPipeCommunicationManager::Connect() {
  namedPipeListener_ = std::make_unique<NamedPipeListener<VRInputData>>(configuration_.pipeName);
  return true;
}

void NamedPipeCommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  callback_ = callback;

  if (!Connect()) {
    DriverLog("Unable to connect to named pipe.");
    return;
  }

  namedPipeListener_->StartListening([&](const VRInputData* data) { callback_(*data); });
}

bool NamedPipeCommunicationManager::DisconnectFromDevice() {
  namedPipeListener_->StopListening();
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