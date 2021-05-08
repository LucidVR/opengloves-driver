#include <Encode/LegacyEncodingManager.h>

#include <sstream>
#include <vector>

#include "DriverLog.h"

VRCommData_t LegacyEncodingManager::Decode(std::string input) {
  std::string buf;
  std::stringstream ss(input);

  std::vector<float> tokens(VRCommDataInputPosition::MAX);
  std::fill(tokens.begin(), tokens.begin() + VRCommDataInputPosition::MAX,
            0.0f);

  try {
    short i = 0;
    while (getline(ss, buf, '&')) {
      tokens[i] = std::stof(buf);
      i++;
    }
  } catch (const std::invalid_argument &ia) {
    DriverLog("Received error trying to decode. Skipping...");
    VRCommData_t data();
  }
  std::array<float, 5> flexion;
  std::array<float, 5> splay;

  for (int i = 0; i < 5; i++) {
    flexion[i] = tokens[i] / m_maxAnalogValue;
    splay[i] = 0.5;
  }

  const float joyX =
      (2 * tokens[VRCommDataInputPosition::JOY_X] / m_maxAnalogValue) - 1;
  const float joyY =
      (2 * tokens[VRCommDataInputPosition::JOY_Y] / m_maxAnalogValue) - 1;

  VRCommData_t commData(flexion, splay, joyX, joyY,
                        tokens[VRCommDataInputPosition::JOY_BTN] == 1,
                        tokens[VRCommDataInputPosition::BTN_TRG] == 1,
                        tokens[VRCommDataInputPosition::BTN_A] == 1,
                        tokens[VRCommDataInputPosition::BTN_B] == 1,
                        tokens[VRCommDataInputPosition::GES_GRAB] == 1,
                        tokens[VRCommDataInputPosition::GES_PINCH] == 1);

  return commData;
}
