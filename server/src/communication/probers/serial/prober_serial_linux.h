#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../../../../include/communication/probers/communication_prober.h"

class SerialCommunicationProber : CommunicationProber {
 public:
  int InquireDevices(std::vector<std::unique_ptr<CommunicationService>>& out_devices) override;

 private:
  std::function<void(std::unique_ptr<CommunicationService>)> callback_;
};