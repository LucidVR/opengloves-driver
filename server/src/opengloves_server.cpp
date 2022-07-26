#include "device_discovery.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

int Server::StartProber(std::function<void(std::unique_ptr<Device> device)> callback) {
  callback_ = callback;

  device_discovery_ = std::make_unique<DeviceDiscovery>(legacy_configuration_, callback_);

  return 0;
}

void Server::SetLegacyConfiguration(const LegacyConfiguration& legacy_configuration) {
  legacy_configuration_ = legacy_configuration;
}

int Server::StopProber() {
  logger.Log(kLoggerLevel_Info, "Stopping device discovery");
  device_discovery_.reset();

  logger.Log(kLoggerLevel_Info, "Successfully stopped device discovery");

  return 0;
}