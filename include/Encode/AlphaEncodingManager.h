#pragma once

#include "Encode/EncodingManager.h"

class AlphaEncodingManager : public EncodingManager {
 public:
  explicit AlphaEncodingManager(float maxAnalogValue);

  VRInputData Decode(std::string input) override;
  std::string Encode(const VRFFBData& input) override;
};