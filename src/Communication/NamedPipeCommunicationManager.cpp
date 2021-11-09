#include "Communication/NamedPipeCommunicationManager.h"

#include <utility>

NamedPipeCommunicationManager::NamedPipeCommunicationManager(
    VRNamedPipeInputConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration)
    : CommunicationManager(deviceConfiguration), _isConnected(false), _configuration(std::move(configuration)){};

bool NamedPipeCommunicationManager::Connect() {
  _namedPipeListener = std::make_unique<NamedPipeListener<VRInputData>>(_configuration.pipeName);
  return true;
}

void NamedPipeCommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  _callback = callback;

  if (!Connect()) {
    DriverLog("Unable to connect to named pipe.");
    return;
  }

  _namedPipeListener->StartListening([&](const VRInputData* data) { _callback(*data); });
}

bool NamedPipeCommunicationManager::DisconnectFromDevice() {
  _namedPipeListener->StopListening();
  return true;
}

bool NamedPipeCommunicationManager::IsConnected() {
  return _namedPipeListener->IsConnected();
}

void NamedPipeCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, _configuration.pipeName.c_str(), GetLastErrorAsString().c_str());
}

void NamedPipeCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, _configuration.pipeName.c_str());
}