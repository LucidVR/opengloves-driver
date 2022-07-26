#pragma once

#include "communication_prober.h"

class BluetoothCommunicationProber : public ICommunicationProber {
 public:
  BluetoothCommunicationProber() = default;

  int InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

  std::string GetName() override {
    return "bluetooth";
  }

 private:
  std::function<void(std::unique_ptr<ICommunicationService>)> callback_;
};