#pragma once

#include <memory>

#include "device_discovery.h"

class LucidglovesNamedPipeDiscovery : public IDeviceDiscoverer {
 public:
  void StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesNamedPipeDiscovery();

 private:
  std::function<void(std::unique_ptr<og::Device> device)> device_discovered_callback_;

  class Impl;
  std::unique_ptr<Impl> pImpl_;
};