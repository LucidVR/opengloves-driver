#include "device_discovery.h"

#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

DeviceDiscovery::DeviceDiscovery(std::function<void(std::unique_ptr<og::Device>*)>& callback) {
  prober_manager_ = std::make_unique<ProberManager>(
      [&](std::unique_ptr<ICommunicationService> communication_service) { OnDeviceDiscovered(std::move(communication_service)); });
}

void DeviceDiscovery::OnDeviceDiscovered(std::unique_ptr<ICommunicationService> communication_service) {
  // request information from the glove
  communication_service->RawWrite("Z\n");

  // flush any extra packets we might have
  int retries = 0;
  do {
    std::string buff;
    communication_service->ReceiveNextPacket(buff);

    communication_service->RawWrite("Z\n");

    retries++;
  } while (retries < 5);
}