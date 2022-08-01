#include "device/discovery/lucidgloves_fw_discovery.h"
#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

class Server::Impl {
 public:
  void SetDefaultConfiguration(const og::DeviceDefaultConfiguration& configuration) {
    default_configuration_ = configuration;
  }

  bool StartProber(const std::function<void(std::unique_ptr<Device> device)>& callback) {
    logger.Log(kLoggerLevel_Info, "Starting server prober...");

    callback_ = callback;

    // lucidgloves firmware discovery (or other firmwares that use the same communication methods and encoding schemes)
    device_discoverers_.emplace_back(std::make_unique<LucidglovesDeviceDiscoverer>(default_configuration_));

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
  std::function<void(std::unique_ptr<Device> device)> callback_;
  std::vector<std::unique_ptr<DeviceDiscoverer>> device_discoverers_;
  DeviceDefaultConfiguration default_configuration_;
};

Server::Server() {
  pImpl_ = std::make_unique<Server::Impl>();
}

void Server::SetDefaultConfiguration(const og::DeviceDefaultConfiguration& configuration) {
  pImpl_->SetDefaultConfiguration(configuration);
}

bool Server::StartProber(std::function<void(std::unique_ptr<Device> device)> callback) {
  return pImpl_->StartProber(callback);
}

bool Server::StopProber() {
  return pImpl_->StopProber();
}

Server::~Server() {
  logger.Log(kLoggerLevel_Info, "Shutting down server");
  StopProber();
}