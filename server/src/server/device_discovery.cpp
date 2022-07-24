#include "device_discovery.h"

#include "probers/serial/prober_serial.h"

int DeviceDiscovery::StartDiscovery(std::function<void(std::unique_ptr<og::Device> *)> &callback) {

  //add probers here
  probers_.emplace_back(std::make_unique<SerialCommunicationProber>());

  for (auto const &prober : probers_) {
	
  }

  return kDeviceDiscoveryError_Success;
}
