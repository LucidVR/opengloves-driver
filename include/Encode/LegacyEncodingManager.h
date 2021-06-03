#pragma once

#include <Encode/EncodingManager.h>
#include "ForceFeedback/FFBPipe.h"

class LegacyEncodingManager : public IEncodingManager {
 public:
  LegacyEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};

  VRCommData_t Decode(std::string input);

  std::string Encode(const VRFFBData_t& input);

 private:
  float m_maxAnalogValue;
};