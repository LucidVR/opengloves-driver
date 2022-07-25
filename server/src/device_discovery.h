#pragma once
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "opengloves_interface.h"
#include "probers/communication_prober.h"

enum DeviceDiscoveryError {
  kDeviceDiscoveryError_Success,
  kDeviceDiscoveryError_CommunicationError,
};

class DeviceDiscovery {
 public:
  DeviceDiscovery(const og::LegacyConfiguration& legacy_configuration, std::function<void(std::unique_ptr<og::Device> device)>& callback);

  ~DeviceDiscovery();

 private:
  std::function<void(std::unique_ptr<og::Device> device)> callback_;

  void OnDeviceDiscovered(std::unique_ptr<ICommunicationService> communication_service);

  std::unique_ptr<ProberManager> prober_manager_;

  og::LegacyConfiguration legacy_configuration_;
};