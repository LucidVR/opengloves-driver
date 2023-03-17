// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "communication/probers/prober.h"
#include "communication/encoding/encoding_service.h"
#include "communication/services/communication_service.h"
#include "opengloves_interface.h"

class LucidglovesDeviceDiscoverer : public og::IDeviceDiscoverer {
 public:
  LucidglovesDeviceDiscoverer(og::CommunicationConfiguration communication_configuration, std::vector<og::DeviceConfiguration> device_configurations);

  void StartDiscovery(std::function<void(std::unique_ptr<og::IDevice> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesDeviceDiscoverer() override;

 private:
  void ProberThread(std::unique_ptr<ICommunicationProber> prober, const std::function<void(std::unique_ptr<ICommunicationService> service)>& callback);
  void OnDeviceFound(const og::DeviceConfiguration& configuration, std::unique_ptr<ICommunicationService> service);

  std::function<void(std::unique_ptr<og::IDevice> device)> callback_;

  std::vector<std::thread> prober_threads_;

  std::mutex device_found_mutex_;

  std::atomic<bool> is_active_;

  std::vector<og::DeviceConfiguration> device_configurations_;
  og::CommunicationConfiguration communication_configuration_;
};