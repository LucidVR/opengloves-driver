#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "communication/probers/communication_prober.h"
#include "communication/services/communication_service.h"
#include "device_discovery.h"
#include "opengloves_interface.h"

class LucidglovesDeviceDiscoverer : public IDeviceDiscoverer {
 public:
  LucidglovesDeviceDiscoverer(og::CommunicationConfiguration communication_configuration, std::vector<og::DeviceConfiguration> device_configurations);

  void StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesDeviceDiscoverer();

 private:
  void ProberThread(std::unique_ptr<ICommunicationProber> prober);
  void OnDeviceFound(std::unique_ptr<ICommunicationService> communication_service, og::CommunicationType communication_type);

  std::function<void(std::unique_ptr<og::Device> device)> callback_;

  std::vector<std::thread> prober_threads_;

  std::mutex device_found_mutex_;

  std::atomic<bool> is_active_;

  std::vector<og::DeviceConfiguration> device_configurations_;
  og::CommunicationConfiguration communication_configuration_;
};