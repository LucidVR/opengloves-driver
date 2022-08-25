#include "named_pipe_communication_manager.h"

NamedPipeCommunicationManager::NamedPipeCommunicationManager(std::string pipe_name, std::function<void()> on_client_connected) {

}


void NamedPipeCommunicationManager::BeginListener(std::function<void(const og::Input &)> callback) {}