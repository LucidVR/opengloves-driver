#pragma once

#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

class AlphaEncodingManager : public EncodingManager {
 public:
  explicit AlphaEncodingManager(const VREncodingConfiguration& configuration) : EncodingManager(configuration){};

  VRInputData Decode(const std::string& input) override;
  std::string Encode(const VROutput& input) override;
};