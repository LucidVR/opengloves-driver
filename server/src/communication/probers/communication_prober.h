#pragma once

#include <memory>
#include <vector>

class CommunicationProber {
  virtual int InquireDevices(std::vector<std::unique_ptr<CommunicationService>>& out_devices) = 0;
};