#pragma once

#include <memory>

#include "opengloves_interface.h"

class LucidglovesNamedPipeDiscovery : public og::IDeviceDiscoverer {
 public:
  void StartDiscovery(std::function<void(std::unique_ptr<og::IDevice> device)> callback) override;

  void StopDiscovery();

  ~LucidglovesNamedPipeDiscovery();

 private:
  std::function<void(std::unique_ptr<og::IDevice> device)> device_discovered_callback_;

  class Impl;
  std::unique_ptr<Impl> pImpl_;
};