#pragma once

#include <memory>
#include <string>
#include <vector>

#include "communication/probers/prober.h"
#include "communication/services/communication_service.h"

struct SerialProberPort {
  std::string port;
};

class SerialPortProber : public ICommunicationProber {
 public:
  explicit SerialPortProber(const SerialProberPort& configuration);

  bool InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;

 private:
  std::string port_;
};