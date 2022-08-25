#pragma once

#include <memory>

#include "device_discovery.h"

class LucidglovesNamedPipeDiscovery : public IDeviceDiscoverer {
 public:
  LucidglovesNamedPipeDiscovery();

  void StartDiscovery(std::function<void(std::unique_ptr<og::Device> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesNamedPipeDiscovery();

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};