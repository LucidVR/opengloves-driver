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
  int StartDiscovery(std::function<void(std::unique_ptr<og::Device>* device)>& callback);

  void DiscoveryProberThread();

  int StopDiscovery();

  ~DeviceDiscovery();

 private:
  std::vector<std::unique_ptr<ICommunicationProber>> probers_;
  std::vector<std::string> active_device_addresses_;

  std::vector<std::thread> prober_threads_;
};