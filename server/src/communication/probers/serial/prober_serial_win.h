#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "probers/communication_prober.h"
#include "services/communication_service.h"

struct SerialProberSearchParam {
  auto operator<=>(const SerialProberSearchParam&) const = default;

  std::string vid;
  std::string pid;
};

class SerialCommunicationProber : public ICommunicationProber {
 public:
  SerialCommunicationProber(const std::vector<SerialProberSearchParam>& params);

  int InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;
  std::string GetName() override;

 private:
  std::function<void(std::unique_ptr<ICommunicationService>)> callback_;

  std::vector<SerialProberSearchParam> params_;
};