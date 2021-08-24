#pragma once

#include "Communication/CommunicationObjects.h"
#include "Encode/EncodingManager.h"

class LegacyEncodingManager : public EncodingManager {
 public:
  LegacyEncodingManager(float maxAnalogValue);

  VRInputData_t Decode(std::string input);
  std::string Encode(const VRFFBData_t& input);
};