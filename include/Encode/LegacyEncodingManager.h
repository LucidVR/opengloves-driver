#pragma once

#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

class LegacyEncodingManager : public EncodingManager {
 public:
  explicit LegacyEncodingManager(const VREncodingConfiguration& configuration) : EncodingManager(configuration){};

  VRInputData Decode(const std::string& input) override;
  std::string Encode(const VROutput& input) override;
};