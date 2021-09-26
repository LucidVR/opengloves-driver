#include "Communication/NamedPipeCommunicationManager.h"

NamedPipeCommunicationManager::NamedPipeCommunicationManager(const VRNamedPipeInputConfiguration& configuration) : m_configuration(configuration), m_isConnected(false){};

bool NamedPipeCommunicationManager::Connect() {
  m_namedPipeListener = std::make_unique<NamedPipeListener<VRInputData>>(m_configuration.pipeName);
  return true;
}

void NamedPipeCommunicationManager::BeginListener(const std::function<void(VRInputData)>& callback) {
  m_namedPipeListener->StartListening([&](VRInputData* data) { callback(*data); });
}

bool NamedPipeCommunicationManager::DisconnectFromDevice() {
  m_namedPipeListener->StopListening();
  return true;
}

bool NamedPipeCommunicationManager::IsConnected() { return m_namedPipeListener->IsConnected(); }

void NamedPipeCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, m_configuration.pipeName.c_str(), GetLastErrorAsString().c_str());
}

void NamedPipeCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, m_configuration.pipeName.c_str());
}