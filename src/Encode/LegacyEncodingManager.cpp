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
  const float trgValue = flexion[1];  // legacy trigger behavior for legacy encoding 

  VRInputData inputData(
      flexion,
      joyX,
      joyY,
      trgValue,
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
  }
}