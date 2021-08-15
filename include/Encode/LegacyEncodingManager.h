#pragma once

#include <Encode/EncodingManager.h>
#include "ForceFeedback.h"

enum class VRCommDataLegacyEncodingPosition : int {
  FIN_THUMB,
  FIN_INDEX,
  FIN_MIDDLE,
  FIN_RING,
  FIN_PINKY,
  JOY_X,
  JOY_Y,
  JOY_BTN,
  BTN_TRG,
  BTN_A,
  BTN_B,
  GES_GRAB,
  GES_PINCH,
  MAX,
};

class LegacyEncodingManager : public IEncodingManager {
 public:
  LegacyEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};

  VRCommData_t Decode(std::string input);

  std::string Encode(const VRFFBData_t& input);

 private:
  float m_maxAnalogValue;
};