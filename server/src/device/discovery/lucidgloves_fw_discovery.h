#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "device_discovery.h"
#include "opengloves_interface.h"
#include "probers/communication_prober.h"
#include "services/communication_service.h"

class LucidglovesDeviceDiscoverer : public DeviceDiscoverer {
 public:
  LucidglovesDeviceDiscoverer(const og::DeviceDefaultConfiguration& default_configuration);

  void StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback);

  void StopDiscovery();

  ~LucidglovesDeviceDiscoverer();

 private:
  void QueryableProberThread(std::unique_ptr<ICommunicationProber> prober);
  void OnQueryableDeviceFound(std::unique_ptr<ICommunicationService> communication_service);

  std::mutex device_found_mutex_;

  std::vector<std::thread> queryable_prober_threads_;

  std::function<void(std::unique_ptr<og::Device> device)> callback_;

  std::atomic<bool> is_active_;

  og::DeviceDefaultConfiguration default_configuration_;
};