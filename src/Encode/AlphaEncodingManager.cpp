#include <Encode/AlphaEncodingManager.h>

#include <sstream>
#include <vector>
#include "DriverLog.h"


/* Alpha encoding uses the wasted data in the delimiter from legacy to allow for optional arguments and redundancy over smaller packets */

std::string AlphaEncodingManager::getArgumentSubstring(std::string str, char del) { 
    size_t start = str.find(del);
    if (start == std::string::npos)
        return std::string();
    size_t end = str.find_first_of(VRCommDataAlphaEncodingCharacters, start + 1);  // characters may not necessarily be in order, so end at any letter
    return str.substr(start + 1, end - (start + 1));
}

bool AlphaEncodingManager::argValid(std::string str, char del) { return str.find(del) != std::string::npos; }

VRCommData_t AlphaEncodingManager::Decode(std::string input) {

    std::array<float, 5> flexion;
    std::array<float, 5> splay;

    for (int i = 0; i < 5; i++) { //splay tracking not yet supported
        flexion[i] = -1; // 0.5;
        splay[i] = 0.5;
    }

    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_THUMB))
      flexion[0] =
          stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_THUMB)) / m_maxAnalogValue;
    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_INDEX))
      flexion[1] =
          stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_INDEX)) / m_maxAnalogValue;
    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE))
      flexion[2] =
          stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE)) / m_maxAnalogValue;
    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_RING))
      flexion[3] =
          stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_RING)) / m_maxAnalogValue;
    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::FIN_PINKY))
      flexion[4] =
          stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::FIN_PINKY)) / m_maxAnalogValue;

    float joyX = 0;
    float joyY = 0;

    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::JOY_X))
      joyX = 2 * stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::JOY_X)) / m_maxAnalogValue - 1;
    if (argValid(input, (char)VRCommDataAlphaEncodingCharacter::JOY_Y))
      joyY = 2 * stof(getArgumentSubstring(input, (char)VRCommDataAlphaEncodingCharacter::JOY_Y)) / m_maxAnalogValue - 1;

    VRCommData_t commData(
        flexion,
        splay,
        joyX,
        joyY,
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::JOY_BTN),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_TRG),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_A),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_B),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::GES_GRAB),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::GES_PINCH),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_MENU),
        argValid(input, (char)VRCommDataAlphaEncodingCharacter::BTN_CALIB)
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

std::string AlphaEncodingManager::Encode(const VRFFBData_t& data) {
  std::string result = string_format("A%dB%dC%dD%dE%d\n", data.thumbCurl, data.indexCurl, data.middleCurl, data.ringCurl, data.pinkyCurl);
  return result;
};