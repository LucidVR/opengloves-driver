#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "device_discovery.h"
#include "opengloves_interface.h"
#include "communication/probers/communication_prober.h"
#include "communication/services/communication_service.h"

class LucidglovesDeviceDiscoverer : public IDeviceDiscoverer {
 public:
  explicit LucidglovesDeviceDiscoverer(const og::DeviceDefaultConfiguration& default_configuration);

  void StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesDeviceDiscoverer();

 private:
  void ProberThread(std::unique_ptr<ICommunicationProber> prober);
  void OnDeviceFound(std::unique_ptr<ICommunicationService> communication_service);

  std::function<void(std::unique_ptr<og::Device> device)> callback_;

  std::vector<std::thread> prober_threads_;

  std::mutex device_found_mutex_;

  std::atomic<bool> is_active_;

  og::DeviceDefaultConfiguration default_configuration_;
};