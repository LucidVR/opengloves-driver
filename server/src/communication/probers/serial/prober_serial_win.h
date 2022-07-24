#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "communication_prober.h"
#include "services/communication_service.h"

class SerialCommunicationProber : public ICommunicationProber {
 public:
  SerialCommunicationProber() = default;

  int InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;
  std::string GetName() override;

 private:
  std::function<void(std::unique_ptr<ICommunicationService>)> callback_;
};