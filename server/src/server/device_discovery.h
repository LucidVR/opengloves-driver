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
  DeviceDiscovery(std::function<void(std::unique_ptr<og::Device>* device)>& callback);

  ~DeviceDiscovery();

 private:
  void OnDeviceDiscovered(std::unique_ptr<ICommunicationService> communication_service);

  std::unique_ptr<ProberManager> prober_manager_;
};