#pragma once

#include "DeviceConfiguration.h"
#include <Encode/EncodingManager.h>
#include "ForceFeedback.h"

#define LEGACY_ENCODING_SETTINGS_SECTION_WITHOUT_PREFIX "encoding_legacy"
#define LEGACY_ENCODING_SETTINGS_SECTION (OPENGLOVES_SECTION_PREFIX LEGACY_ENCODING_SETTINGS_SECTION_WITHOUT_PREFIX)

class LegacyEncodingManager : public IEncodingManager {
 public:
  LegacyEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};

  VRCommData_t Decode(std::string input);

  std::string Encode(const VRFFBData_t& input);

 private:
  float m_maxAnalogValue;
};