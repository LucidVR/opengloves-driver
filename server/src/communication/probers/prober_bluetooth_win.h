#pragma once

#include <string>
#include <vector>

#include "communication/probers/communication_prober.h"
#include "opengloves_interface.h"

struct BluetoothProberConfiguration {
  std::vector<std::string> identifiers;
};

class BluetoothCommunicationProber : public ICommunicationProber {
 public:
  explicit BluetoothCommunicationProber(BluetoothProberConfiguration configuration);

  og::CommunicationType InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

  std::string GetName() override {
    return "bluetooth";
  }

 private:
  std::function<void(std::unique_ptr<ICommunicationService>)> callback_;

  BluetoothProberConfiguration configuration_;
};