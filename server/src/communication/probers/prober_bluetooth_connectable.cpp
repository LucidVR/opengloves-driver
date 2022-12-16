#include "prober_bluetooth_connectable.h"

#include <utility>

#ifdef _WIN32
#include "communication/services/service_bluetooth_win.h"
#endif
#ifdef linux
#include "communication/services/service_bluetooth_linux.h"
#endif

BluetoothPortProber::BluetoothPortProber(BluetoothPortProberConfiguration configuration) : configuration_(std::move(configuration)) {}

bool BluetoothPortProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>> &out_devices) {
  og::DeviceBluetoothCommunicationConfiguration service_configuration{configuration_.port};
  std::unique_ptr<BluetoothCommunicationService> bluetooth_service = std::make_unique<BluetoothCommunicationService>(service_configuration);

  if (bluetooth_service->IsConnected()) {
    out_devices.emplace_back(std::move(bluetooth_service));

    return true;
  }

  return false;
}