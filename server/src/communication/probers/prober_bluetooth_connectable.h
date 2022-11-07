#pragma once

#include "prober.h"

struct BluetoothPortProberConfiguration {
  std::string port;
};

class BluetoothPortProber : public ICommunicationProber {
 public:
  explicit BluetoothPortProber(BluetoothPortProberConfiguration  configuration);

  bool InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

 private:
  BluetoothPortProberConfiguration configuration_;
};