#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "communication/probers/communication_prober.h"
#include "communication/services/communication_service.h"
#include "opengloves_interface.h"

struct SerialProberIdentifier {
  std::string vid;
  std::string pid;
};

struct SerialProberConfiguration {
  std::vector<SerialProberIdentifier> identifiers;
};

class SerialCommunicationProber : public ICommunicationProber {
 public:
  explicit SerialCommunicationProber(SerialProberConfiguration configuration);

  og::CommunicationType InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) override;
  std::string GetName() override;

 private:
  std::function<void(std::unique_ptr<ICommunicationService>)> callback_;

  std::vector<std::string> identifiers_;

  SerialProberConfiguration configuration_;
};