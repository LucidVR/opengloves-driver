#include "lucidgloves_fw_discovery.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

void Server::SetLegacyConfiguration(const LegacyConfiguration& legacy_configuration) {
  legacy_configuration_ = legacy_configuration;
}

int Server::StartProber(std::function<void(std::unique_ptr<Device> device)> callback) {
  callback_ = callback;

  // lucidgloves firmware discovery (or other firmwares that use the same communication methods and encoding schemes)
  device_discoverers_.emplace_back(std::make_unique<LucidglovesDeviceDiscoverer>(legacy_configuration_));

  for (auto& discoverer : device_discoverers_) {
    discoverer->StartDiscovery(callback);
  }

  return 0;
}

int Server::StopProber() {
  logger.Log(kLoggerLevel_Info, "Stopping device discovery");

  for (auto& discoverer : device_discoverers_) {
    discoverer.reset();
  }

  logger.Log(kLoggerLevel_Info, "Successfully stopped device discovery");

  return 0;
}