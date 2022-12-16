#include "prober_serial_connectable.h"

#ifdef _WIN32
#include "communication/services/service_serial_win.h"
#endif
#ifdef linux
#include "communication/services/service_serial_linux.h"
#endif

#include "opengloves_interface.h"

SerialPortProber::SerialPortProber(const SerialPortProberConfiguration& configuration) {
  port_ = configuration.port;
}

bool SerialPortProber::InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) {
  og::DeviceSerialCommunicationConfiguration config{port_};
  auto device = std::make_unique<SerialCommunicationService>(config);
  if (device->IsConnected()) {
    out_devices.push_back(std::move(device));
    return true;
  }

  return false;
}