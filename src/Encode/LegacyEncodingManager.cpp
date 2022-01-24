#include <Encode/LegacyEncodingManager.h>

#include <sstream>
#include <vector>

enum class VRCommDataLegacyEncodingPosition : int {
  FinThumb,
  FinIndex,
  FinMiddle,
  FinRing,
  FinPinky,
  JoyX,
  JoyY,
  JoyBtn,
  BtnTrg,
  BtnA,
  BtnB,
  GesGrab,
  GesPinch,
  Max,
};

LegacyEncodingManager::LegacyEncodingManager(const float maxAnalogValue) : EncodingManager(maxAnalogValue) {}

VRInputData LegacyEncodingManager::Decode(const std::string& input) {
  std::string buf;
  std::stringstream ss(input);

  std::vector tokens(static_cast<int>(VRCommDataLegacyEncodingPosition::Max), 0.0f);

  uint64_t tokenI = 0;
  while (tokenI < tokens.size() && getline(ss, buf, '&')) {
    tokens[tokenI] = std::stof(buf);
    tokenI++;
  }

  std::array<float, 5> flexion{};
  for (uint8_t flexionI = 0; flexionI < 5; flexionI++) {
    flexion[flexionI] = tokens[flexionI] / maxAnalogValue_;
  }

  const float joyX = 2 * tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::JoyX)] / maxAnalogValue_ - 1;
  const float joyY = 2 * tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::JoyY)] / maxAnalogValue_ - 1;

  VRInputData inputData(
      flexion,
      joyX,
      joyY,
      tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::JoyBtn)] == 1,
      tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::BtnTrg)] == 1,
      tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::BtnA)] == 1,
      tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::BtnB)] == 1,
      tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::GesGrab)] == 1,
      tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::GesPinch)] == 1,
      false,
      false);

  return inputData;
}

std::string LegacyEncodingManager::Encode(const VRFFBData& input) {
  std::string result = StringFormat("%d&%d&%d&%d&%d\n", input.thumbCurl, input.indexCurl, input.middleCurl, input.ringCurl, input.pinkyCurl);
  return result;
}