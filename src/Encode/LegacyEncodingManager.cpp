#include <Encode/LegacyEncodingManager.h>

#include <sstream>
#include <vector>

#include "DriverLog.h"

VRCommData_t LegacyEncodingManager::Decode(std::string input) {
  std::string buf;
  std::stringstream ss(input);

  std::vector<float> tokens(VRCommDataInputPosition::MAX);
  std::fill(tokens.begin(), tokens.begin() + VRCommDataInputPosition::MAX, 0.0f);

  short i = 0;
  while (getline(ss, buf, '&')) {
    tokens[i] = std::stof(buf);
    i++;
  }

  std::array<float, 5> flexion;
  std::array<float, 5> splay;

  for (int i = 0; i < 5; i++) {
    flexion[i] = tokens[i] / m_maxAnalogValue;
    splay[i] = 0.5;
  }

  const float joyX = (2 * tokens[VRCommDataInputPosition::JOY_X] / m_maxAnalogValue) - 1;
  const float joyY = (2 * tokens[VRCommDataInputPosition::JOY_Y] / m_maxAnalogValue) - 1;

    VRCommData_t commData(
        flexion,
        splay,
        joyX,
        joyY,
        tokens[VRCommDataInputPosition::JOY_BTN] == 1,
        tokens[VRCommDataInputPosition::BTN_TRG] == 1,
        tokens[VRCommDataInputPosition::BTN_A] == 1,
        tokens[VRCommDataInputPosition::BTN_B] == 1,
        tokens[VRCommDataInputPosition::GES_GRAB] == 1,
        tokens[VRCommDataInputPosition::GES_PINCH] == 1,
        false
    );

  return commData;
}

template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
  if (size_s <= 0) {
    DriverLog("Error decoding string");
    return "";
  }
  auto size = static_cast<size_t>(size_s);
  auto buf = std::make_unique<char[]>(size);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}

std::string LegacyEncodingManager::Encode(const VRFFBData_t& data) {
  std::string result = string_format("%d&%d&%d&%d&%d\n", data.thumbCurl, data.indexCurl, data.middleCurl, data.ringCurl, data.pinkyCurl);
  return result;
};