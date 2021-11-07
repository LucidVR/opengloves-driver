#pragma once

#include "Encode/EncodingManager.h"

class LegacyEncodingManager : public EncodingManager {
 public:
  LegacyEncodingManager(float maxAnalogValue);

  VRInputData Decode(std::string input);
  std::string Encode(const VRFFBData& input);
};