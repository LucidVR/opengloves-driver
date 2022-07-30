#include "lucidgloves_fw_discovery.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

void Server::SetDefaultConfiguration(const og::DeviceDefaultConfiguration& configuration) {
  default_configuration_ = configuration;
}

int Server::StartProber(std::function<void(std::unique_ptr<Device> device)> callback) {
  callback_ = callback;

  // lucidgloves firmware discovery (or other firmwares that use the same communication methods and encoding schemes)
  device_discoverers_.emplace_back(std::make_unique<LucidglovesDeviceDiscoverer>(default_configuration_));

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