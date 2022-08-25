#pragma once

#include "communication_manager.h"

#include "opengloves_interface.h"

class NamedPipeCommunicationManager : public ICommunicationManager {
 public:
  NamedPipeCommunicationManager(std::string pipe_name, std::function<void()> on_client_connected);

  void BeginListener(std::function<void(const og::Input&)> callback) override;

 private:
  std::string pipe_name_;
  std::function<void()> on_client_connected;
};