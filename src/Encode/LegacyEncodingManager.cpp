#include <Encode/LegacyEncodingManager.h>

#include <sstream>
#include <vector>

#include "DriverLog.h"

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

LegacyEncodingManager::LegacyEncodingManager(float maxAnalogValue) : EncodingManager(maxAnalogValue) {}

VRInputData LegacyEncodingManager::Decode(std::string input) {
  std::string buf;
  std::stringstream ss(input);

  std::vector<float> tokens((int)VRCommDataLegacyEncodingPosition::MAX, 0.0f);

  short i = 0;
  while (i < tokens.size() && getline(ss, buf, '&')) {
    tokens[i] = std::stof(buf);
    i++;
  }

  std::array<float, 5> flexion{};
  for (int i = 0; i < 5; i++) {
    flexion[i] = tokens[i] / m_maxAnalogValue;
  }

  const float joyX = (2 * tokens[(int)VRCommDataLegacyEncodingPosition::JOY_X] / m_maxAnalogValue) - 1;
  const float joyY = (2 * tokens[(int)VRCommDataLegacyEncodingPosition::JOY_Y] / m_maxAnalogValue) - 1;

  VRInputData inputData(flexion, joyX, joyY, tokens[(int)VRCommDataLegacyEncodingPosition::JOY_BTN] == 1, tokens[(int)VRCommDataLegacyEncodingPosition::BTN_TRG] == 1,
                          tokens[(int)VRCommDataLegacyEncodingPosition::BTN_A] == 1, tokens[(int)VRCommDataLegacyEncodingPosition::BTN_B] == 1,
                          tokens[(int)VRCommDataLegacyEncodingPosition::GES_GRAB] == 1, tokens[(int)VRCommDataLegacyEncodingPosition::GES_PINCH] == 1, false, false);

  return inputData;
}

std::string LegacyEncodingManager::Encode(const VRFFBData& data) {
  std::string result = string_format("%d&%d&%d&%d&%d\n", data.thumbCurl, data.indexCurl, data.middleCurl, data.ringCurl, data.pinkyCurl);
  return result;
};