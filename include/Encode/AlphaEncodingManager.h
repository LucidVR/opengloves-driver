#pragma once

#include "Encode/EncodingManager.h"

class AlphaEncodingManager : public EncodingManager {
 public:
  AlphaEncodingManager(float maxAnalogValue);

  VRInputData Decode(std::string input);
  std::string Encode(const VRFFBData& input);
};