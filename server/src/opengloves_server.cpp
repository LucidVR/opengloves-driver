// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include <utility>

#include "device/lucidgloves/discovery/lucidgloves_fw_discovery.h"
#include "device/lucidgloves/discovery/lucidgloves_named_pipe_discovery.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

class Server::Impl {
 public:
  explicit Impl(ServerConfiguration configuration) : configuration_(std::move(configuration)){};

  bool StartProber(const std::function<void(std::unique_ptr<IDevice> device)>& callback) {
    logger.Log(kLoggerLevel_Info, "Starting server prober...");

    callback_ = callback;

    // lucidgloves firmware discovery (or other firmwares that use the same communication methods and encoding schemes)
    device_discoverers_.emplace_back(std::make_unique<LucidglovesDeviceDiscoverer>(configuration_.communication, configuration_.devices));
    if (configuration_.communication.named_pipe.enabled) device_discoverers_.emplace_back(std::make_unique<LucidglovesNamedPipeDiscovery>());

    for (auto& discoverer : device_discoverers_) {
      discoverer->StartDiscovery(callback);
    }

    return true;
  }

  bool StopProber() {
    logger.Log(kLoggerLevel_Info, "Stopping device discovery");

    for (auto& discoverer : device_discoverers_) {
      discoverer.reset();
    }

    logger.Log(kLoggerLevel_Info, "Successfully stopped device discovery");

    return true;
  }

 private:
  std::function<void(std::unique_ptr<IDevice> device)> callback_;
  std::vector<std::unique_ptr<IDeviceDiscoverer>> device_discoverers_;

  ServerConfiguration configuration_;
};

Server::Server(ServerConfiguration configuration) {
  pImpl_ = std::make_unique<Server::Impl>(std::move(configuration));
}

bool Server::StartProber(std::function<void(std::unique_ptr<IDevice> device)> callback) {
  return pImpl_->StartProber(callback);
}

bool Server::StopProber() {
  return pImpl_->StopProber();
}

Server::~Server() {
  logger.Log(kLoggerLevel_Info, "Shutting down server");
  StopProber();
}