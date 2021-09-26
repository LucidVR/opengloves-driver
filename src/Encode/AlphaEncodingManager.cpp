#include <Encode/AlphaEncodingManager.h>

#include <sstream>
#include <vector>

#include "DriverLog.h"

/* Alpha encoding uses the wasted data in the delimiter from legacy to allow for optional arguments and redundancy over smaller packets */
enum class VRCommDataAlphaEncodingCharacter : char {
  FIN_THUMB = 'A',
  FIN_INDEX = 'B',
  FIN_MIDDLE = 'C',
  FIN_RING = 'D',
  FIN_PINKY = 'E',
  JOY_X = 'F',
  JOY_Y = 'G',
  JOY_BTN = 'H',
  BTN_TRG = 'I',
  BTN_A = 'J',
  BTN_B = 'K',
  GES_GRAB = 'L',
  GES_PINCH = 'M',
  BTN_MENU = 'N',
  BTN_CALIB = 'O',
};

const char VRCommDataAlphaEncodingCharacters[] = {
    (char)VRCommDataAlphaEncodingCharacter::FIN_THUMB,
    (char)VRCommDataAlphaEncodingCharacter::FIN_INDEX,
    (char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE,
    (char)VRCommDataAlphaEncodingCharacter::FIN_RING,
    (char)VRCommDataAlphaEncodingCharacter::FIN_PINKY,
    (char)VRCommDataAlphaEncodingCharacter::JOY_X,
    (char)VRCommDataAlphaEncodingCharacter::JOY_Y,
    (char)VRCommDataAlphaEncodingCharacter::JOY_BTN,
    (char)VRCommDataAlphaEncodingCharacter::BTN_TRG,
    (char)VRCommDataAlphaEncodingCharacter::BTN_A,
    (char)VRCommDataAlphaEncodingCharacter::BTN_B,
    (char)VRCommDataAlphaEncodingCharacter::GES_GRAB,
    (char)VRCommDataAlphaEncodingCharacter::GES_PINCH,
    (char)VRCommDataAlphaEncodingCharacter::BTN_MENU,
    (char)VRCommDataAlphaEncodingCharacter::BTN_CALIB,
    (char)0  // Turns into a null terminated string
};

static std::string getArgumentSubstring(std::string str, char del) {
  size_t start = str.find(del);
  if (start == std::string::npos) return std::string();
  size_t end = str.find_first_of(VRCommDataAlphaEncodingCharacters, start + 1);  // characters may not necessarily be in order, so end at any letter
  return str.substr(start + 1, end - (start + 1));
}

static bool argValid(std::string str, char del) { return str.find(del) != std::string::npos; }

AlphaEncodingManager::AlphaEncodingManager(float maxAnalogValue) : EncodingManager(maxAnalogValue) {}

VRInputData AlphaEncodingManager::Decode(std::string input) {
  std::array<float, 5> flexion = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_THUMB))
    flexion[0] = stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_THUMB)) / m_maxAnalogValue;
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_INDEX))
    flexion[1] = stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_INDEX)) / m_maxAnalogValue;
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE))
    flexion[2] = stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE)) / m_maxAnalogValue;
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_RING))
    flexion[3] = stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_RING)) / m_maxAnalogValue;
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_PINKY))
    flexion[4] = stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_PINKY)) / m_maxAnalogValue;

  float joyX = 0;
  float joyY = 0;
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::JOY_X))
    joyX = 2 * stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::JOY_X)) / m_maxAnalogValue - 1;
  if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::JOY_Y))
    joyY = 2 * stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::JOY_Y)) / m_maxAnalogValue - 1;

  VRInputData inputData(flexion, joyX, joyY, argValid(input, (char)VRCommDataAlphaEncodingCharacter::JOY_BTN),
                          argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_TRG), argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_A),
                          argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_B), argValid(input, (char)VRCommDataAlphaEncodingCharacter::GES_GRAB),
                          argValid(input, (char)VRCommDataAlphaEncodingCharacter::GES_PINCH), argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_MENU),
                          argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_CALIB));

  return inputData;
}

std::string AlphaEncodingManager::Encode(const VRFFBData& data) {
  std::string result =
      string_format("%c%d%c%d%c%d%c%d%c%d\n", (char)VRCommDataAlphaEncodingCharacter::FIN_THUMB, data.thumbCurl, (char)VRCommDataAlphaEncodingCharacter::FIN_INDEX,
                    data.indexCurl, (char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE, data.middleCurl, (char)VRCommDataAlphaEncodingCharacter::FIN_RING, data.ringCurl,
                    (char)VRCommDataAlphaEncodingCharacter::FIN_PINKY, data.pinkyCurl);
  return result;
};