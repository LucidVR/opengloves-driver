#include "service_serial_win.h"

SerialCommunicationService::SerialCommunicationService(const std::string &port_name) {
  port_name_ = port_name;
}

int SerialCommunicationService::Connect() {
  is_connected_ = false;

  handle_ = CreateFile(port_name_.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if(handle_ ==)
}
