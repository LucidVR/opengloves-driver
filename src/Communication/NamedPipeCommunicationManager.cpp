#include "Communication/NamedPipeCommunicationManager.h"

#include <regex>
#include <utility>

NamedPipeCommunicationManager::NamedPipeCommunicationManager(
    VRNamedPipeInputConfiguration configuration, const VRDriverConfiguration& deviceConfiguration)
    : CommunicationManager(deviceConfiguration), isConnected_(false), configuration_(std::move(configuration)){};

bool NamedPipeCommunicationManager::Connect() {
  namedPipeListeners_.emplace_back(std::make_unique<NamedPipeListener<VRInputDataVersion::v1>>(
      std::regex_replace(configuration_.pipeName, std::regex("\\$version"), "v1"),
      [&](VRInputDataVersion::v1* data) { callback_(static_cast<VRInputData>(*data)); }));

    namedPipeListeners_.emplace_back(std::make_unique<NamedPipeListener<VRInputDataVersion::v2>>(
      std::regex_replace(configuration_.pipeName, std::regex("\\$version"), "v2"),
      [&](VRInputDataVersion::v2* data) { callback_(static_cast<VRInputData>(*data)); }));
  return true;
}

void NamedPipeCommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  callback_ = callback;

  if (!Connect()) {
    DriverLog("Unable to connect to named pipe.");
    return;
  }

  for (const auto& listener : namedPipeListeners_) {
    listener->StartListening();
  }
}

bool NamedPipeCommunicationManager::DisconnectFromDevice() {
  for (const auto& listener : namedPipeListeners_) {
    listener->StopListening();
  }
  return true;
}

bool NamedPipeCommunicationManager::IsConnected() {
  return true;
}

void NamedPipeCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, configuration_.pipeName.c_str(), GetLastErrorAsString().c_str());
}

void NamedPipeCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, configuration_.pipeName.c_str());
}