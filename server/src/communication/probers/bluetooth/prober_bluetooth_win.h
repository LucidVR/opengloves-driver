#pragma once

#include <string>
#include <vector>

#include "probers/communication_prober.h"

class BluetoothCommunicationProber : public ICommunicationProber {
 public:
  BluetoothCommunicationProber(std::vector<std::string> wanted_devices);

  int InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

  std::string GetName() override {
    return "bluetooth";
  }

 private:
  std::function<void(std::unique_ptr<ICommunicationService>)> callback_;

  std::vector<std::string> wanted_devices_;
};