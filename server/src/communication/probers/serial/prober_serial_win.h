#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class SerialCommunicationProber : public CommunicationProber {
 public:
  SerialCommunicationProber() = default;

  int InquireDevices(std::vector<std::unique_ptr<CommunicationService>>& out_devices) override;

  std::string GetName() override;

 private:
  std::function<void(std::unique_ptr<CommunicationService>)> callback_;
};