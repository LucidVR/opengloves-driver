#include <Encode/AlphaEncodingManager.h>

#include <sstream>

/* Alpha encoding uses the wasted data in the delimiter from legacy to allow for optional arguments and redundancy over smaller packets */
enum class VRCommDataAlphaEncodingCharacter : char {
  FinThumb = 'A',
  FinIndex = 'B',
  FinMiddle = 'C',
  FinRing = 'D',
  FinPinky = 'E',
  JoyX = 'F',
  JoyY = 'G',
  JoyBtn = 'H',
  BtnTrg = 'I',
  BtnA = 'J',
  BtnB = 'K',
  GesGrab = 'L',
  GesPinch = 'M',
  BtnMenu = 'N',
  BtnCalib = 'O',
};

constexpr char VRCommDataAlphaEncodingCharacters[] = {
    static_cast<char>(VRCommDataAlphaEncodingCharacter::FinThumb),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::FinIndex),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::FinMiddle),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::FinRing),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::FinPinky),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyX),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyY),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyBtn),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnTrg),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnA),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnB),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::GesGrab),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::GesPinch),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnMenu),
    static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnCalib),
    static_cast<char>(0  // Turns into a null terminated string
                      )  // Turns into a null terminated string
};

static std::string getArgumentSubstring(const std::string& str, const char del) {
  const size_t start = str.find(del);

  if (start == std::string::npos) return std::string();

  const size_t end =
      str.find_first_of(VRCommDataAlphaEncodingCharacters, start + 1);  // characters may not necessarily be in order, so end at any letter

  return str.substr(start + 1, end - (start + 1));
}

static bool argValid(const std::string& str, const char del) {
  return str.find(del) != std::string::npos;
}

AlphaEncodingManager::AlphaEncodingManager(const float maxAnalogValue) : EncodingManager(maxAnalogValue) {}

VRInputData AlphaEncodingManager::Decode(const std::string input) {
  std::array<float, 5> flexion = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinThumb)))
    flexion[0] = stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinThumb))) / maxAnalogValue_;
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinIndex)))
    flexion[1] = stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinIndex))) / maxAnalogValue_;
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinMiddle)))
    flexion[2] = stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinMiddle))) / maxAnalogValue_;
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinRing)))
    flexion[3] = stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinRing))) / maxAnalogValue_;
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinPinky)))
    flexion[4] = stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::FinPinky))) / maxAnalogValue_;

  float joyX = 0;
  float joyY = 0;
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyX)))
    joyX = 2 * stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyX))) / maxAnalogValue_ - 1;
  if (argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyY)))
    joyY = 2 * stof(getArgumentSubstring(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyY))) / maxAnalogValue_ - 1;

  VRInputData inputData(
      flexion,
      joyX,
      joyY,
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::JoyBtn)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnTrg)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnA)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnB)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::GesGrab)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::GesPinch)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnMenu)),
      argValid(input, static_cast<char>(VRCommDataAlphaEncodingCharacter::BtnCalib)));

  return inputData;
}

std::string AlphaEncodingManager::Encode(const VRFFBData& input) {
  std::string result = StringFormat(
      "%c%d%c%d%c%d%c%d%c%d\n",
      static_cast<char>(VRCommDataAlphaEncodingCharacter::FinThumb),
      input.thumbCurl,
      static_cast<char>(VRCommDataAlphaEncodingCharacter::FinIndex),
      input.indexCurl,
      static_cast<char>(VRCommDataAlphaEncodingCharacter::FinMiddle),
      input.middleCurl,
      static_cast<char>(VRCommDataAlphaEncodingCharacter::FinRing),
      input.ringCurl,
      static_cast<char>(VRCommDataAlphaEncodingCharacter::FinPinky),
      input.pinkyCurl);
  return result;
}