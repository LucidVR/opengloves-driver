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
  Max
};

VRInputData LegacyEncodingManager::Decode(const std::string& input) {
  VRInputData result;

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
    for (int k = 0; k < 4; k++) result.flexion[flexionI][k] = tokens[flexionI] / configuration_.maxAnalogValue;
  }

  result.joyX = 2 * tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::JoyX)] / configuration_.maxAnalogValue - 1;
  result.joyY = 2 * tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::JoyY)] / configuration_.maxAnalogValue - 1;

  result.joyButton = tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::JoyBtn)] == 1;

  result.trgButton = tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::BtnTrg)] == 1;
  result.aButton = tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::BtnA)] == 1;
  result.bButton = tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::BtnB)] == 1;
  result.grab = tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::GesGrab)] == 1;
  result.pinch = tokens[static_cast<int>(VRCommDataLegacyEncodingPosition::GesPinch)] == 1;

  return result;
}

std::string LegacyEncodingManager::Encode(const VROutput& input) {
  switch (input.type) {
    case VROutputDataType::ForceFeedback: {
      const VRFFBData& data = input.data.ffbData;
      return StringFormat("%d&%d&%d&%d&%d", data.thumbCurl, data.indexCurl, data.middleCurl, data.ringCurl, data.pinkyCurl);
    }

    case VROutputDataType::Haptic: {
      const VRHapticData& data = input.data.hapticData;
      return StringFormat("%.2f&%.2f&%.2f&", data.duration, data.frequency, data.amplitude);
    }

    default:
      return "";
  }
}