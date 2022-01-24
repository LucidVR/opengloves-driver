#include <Encode/AlphaEncodingManager.h>
#include <ctype.h>

#include <map>
#include <sstream>

static enum class VRCommDataAlphaEncodingKey : int {
  FinSplayThumb,
  FinSplayIndex,
  FinSplayMiddle,
  FinSplayRing,
  FinSplayPinky,

  FinJointThumb0,
  FinJointThumb1,
  FinJointThumb2,
  FinJointThumb3,  // unused in input but used for parity to other fingers in the array
  FinJointIndex0,
  FinJointIndex1,
  FinJointIndex2,
  FinJointIndex3,
  FinJointMiddle0,
  FinJointMiddle1,
  FinJointMiddle2,
  FinJointMiddle3,
  FinJointRing0,
  FinJointRing1,
  FinJointRing2,
  FinJointRing3,
  FinJointPinky0,
  FinJointPinky1,
  FinJointPinky2,
  FinJointPinky3,

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

  BtnMenu,
  BtnCalib,

  Null,
};

static const std::string keyCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ()";
static bool IsCharacterKeyCharacter(const char character) {
  return keyCharacters.find(character) != std::string::npos;
}

static const std::map<std::string, VRCommDataAlphaEncodingKey> VRCommDataAlphaEncodingInputKeyString{
    {"A", VRCommDataAlphaEncodingKey::FinThumb},   // whole thumb curl (default curl value for thumb joints)
    {"B", VRCommDataAlphaEncodingKey::FinIndex},   // whole index curl (default curl value for index joints)
    {"C", VRCommDataAlphaEncodingKey::FinMiddle},  // whole middle curl (default curl value for middle joints)
    {"D", VRCommDataAlphaEncodingKey::FinRing},    // whole ring curl (default curl value for ring joints)
    {"E", VRCommDataAlphaEncodingKey::FinPinky},   // whole pinky curl (default curl value for pinky joints)

    {"(AAA)", VRCommDataAlphaEncodingKey::FinJointThumb0},   // thumb joint 0
    {"(AAB)", VRCommDataAlphaEncodingKey::FinJointThumb1},   // thumb joint 1
    {"(AAC)", VRCommDataAlphaEncodingKey::FinJointThumb2},   // thumb joint 2
    {"(BAA)", VRCommDataAlphaEncodingKey::FinJointIndex0},   // index joint 0
    {"(BAB)", VRCommDataAlphaEncodingKey::FinJointIndex1},   // index joint 1
    {"(BAC)", VRCommDataAlphaEncodingKey::FinJointIndex2},   // index joint 2
    {"(BAD)", VRCommDataAlphaEncodingKey::FinJointIndex3},   // index joint 3
    {"(CAA)", VRCommDataAlphaEncodingKey::FinJointMiddle0},  // middle joint 0
    {"(CAB)", VRCommDataAlphaEncodingKey::FinJointMiddle1},  // middle joint 1
    {"(CAC)", VRCommDataAlphaEncodingKey::FinJointMiddle2},  // middle joint 2
    {"(CAD)", VRCommDataAlphaEncodingKey::FinJointMiddle3},  // middle joint 3
    {"(DAA)", VRCommDataAlphaEncodingKey::FinJointRing0},    // ring joint 0
    {"(DAB)", VRCommDataAlphaEncodingKey::FinJointRing1},    // ring joint 1
    {"(DAC)", VRCommDataAlphaEncodingKey::FinJointRing2},    // ring joint 2
    {"(DAD)", VRCommDataAlphaEncodingKey::FinJointRing3},    // ring joint 3
    {"(EAA)", VRCommDataAlphaEncodingKey::FinJointPinky0},   // pinky joint 0
    {"(EAB)", VRCommDataAlphaEncodingKey::FinJointPinky1},   // pinky joint 1
    {"(EAC)", VRCommDataAlphaEncodingKey::FinJointPinky2},   // pinky joint 2
    {"(EAD)", VRCommDataAlphaEncodingKey::FinJointPinky3},   // pinky joint 3

    {"(AB)", VRCommDataAlphaEncodingKey::FinSplayThumb},   // whole thumb splay
    {"(BB)", VRCommDataAlphaEncodingKey::FinSplayIndex},   // whole index splay
    {"(CB)", VRCommDataAlphaEncodingKey::FinSplayMiddle},  // whole middle splay
    {"(DB)", VRCommDataAlphaEncodingKey::FinSplayRing},    // whole ring splay
    {"(EB)", VRCommDataAlphaEncodingKey::FinSplayPinky},   // whole pinky splay

    {"F", VRCommDataAlphaEncodingKey::JoyX},      // joystick x component
    {"G", VRCommDataAlphaEncodingKey::JoyY},      // joystick y component
    {"H", VRCommDataAlphaEncodingKey::JoyBtn},    // joystick button
    {"I", VRCommDataAlphaEncodingKey::BtnTrg},    // trigger button
    {"J", VRCommDataAlphaEncodingKey::BtnA},      // A button
    {"K", VRCommDataAlphaEncodingKey::BtnB},      // B button
    {"L", VRCommDataAlphaEncodingKey::GesGrab},   // grab gesture (boolean)
    {"M", VRCommDataAlphaEncodingKey::GesPinch},  // pinch gesture (boolean)
    {"N", VRCommDataAlphaEncodingKey::BtnMenu},   // system button pressed (opens SteamVR menu)
    {"O", VRCommDataAlphaEncodingKey::BtnCalib},  // calibration button
    {"", VRCommDataAlphaEncodingKey::Null},       // Junk key
};

static const std::map<VRCommDataAlphaEncodingKey, std::string> VRCommDataAlphaEncodingOutputKeyString{
    {VRCommDataAlphaEncodingKey::FinThumb, "A"},   // thumb force feedback
    {VRCommDataAlphaEncodingKey::FinIndex, "B"},   // index force feedback
    {VRCommDataAlphaEncodingKey::FinMiddle, "C"},  // middle force feedback
    {VRCommDataAlphaEncodingKey::FinRing, "D"},    // ring force feedback
    {VRCommDataAlphaEncodingKey::FinPinky, "E"}    // pinky force feedback
};

static std::map<VRCommDataAlphaEncodingKey, std::string> ParseInputToMap(const std::string& str) {
  std::map<VRCommDataAlphaEncodingKey, std::string> result;

  int i = 0;
  while (i < str.length()) {
    // Advance until we get an alphabetic character (no point in looking at values that don't have a key associated with them)

    if (str[i] >= 0 && str[i] <= 255 && IsCharacterKeyCharacter(str[i])) {
      std::string key = {str[i]};
      i++;

      // we're going to be parsing a "long key", i.e. (AB) for thumb finger splay. Long keys must always be enclosed in brackets
      if (key[0] == '(') {
        while (str[i] >= 0 && str[i] <= 255 && IsCharacterKeyCharacter(str[i]) && i < str.length()) {
          key += str[i];
          i++;
        }
      }

      std::string value = "";
      while (str[i] >= 0 && str[i] <= 255 && isdigit(str[i]) && i < str.length()) {
        value += str[i];
        i++;
      }

      // Even if the value is empty we still want to use the key, it means that we have a button that is pressed (it only appears in the packet if it
      // is)
      if (VRCommDataAlphaEncodingInputKeyString.find(key) != VRCommDataAlphaEncodingInputKeyString.end())
        result.insert_or_assign(VRCommDataAlphaEncodingInputKeyString.at(key), value);
      else
        DriverLog("Unable to insert key: %s into input map as it was not found", key.c_str());
    } else
      i++;
  }

  return result;
}

VRInputData AlphaEncodingManager::Decode(const std::string& input) {
  std::array<float, 5> flexion = {-1.0f, -1.0f, -1.0f, -1.0f, -1.0f};
  std::array<std::array<float, 4>, 5> jointFlexion;

  std::array<float, 5> splay = {-2.0f, -2.0f, -2.0f, -2.0f, -2.0f};

  // This map contains all the inputs we've got from the packet we received
  std::map<VRCommDataAlphaEncodingKey, std::string> inputMap = ParseInputToMap(input);

  // curl is 0.0f -> 1.0f inclusive
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinThumb) != inputMap.end())
    flexion[0] = std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinThumb)) / maxAnalogValue_;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinIndex) != inputMap.end())
    flexion[1] = std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinIndex)) / maxAnalogValue_;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinMiddle) != inputMap.end())
    flexion[2] = std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinMiddle)) / maxAnalogValue_;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinRing) != inputMap.end())
    flexion[3] = std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinRing)) / maxAnalogValue_;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinPinky) != inputMap.end())
    flexion[4] = std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinPinky)) / maxAnalogValue_;

  // fill all the joints
  int curJoint = (int)VRCommDataAlphaEncodingKey::FinJointThumb0;
  for (int i = 0; i < 5; i++) {
    for (int k = 0; k < 4; k++) {
      VRCommDataAlphaEncodingKey joint = static_cast<VRCommDataAlphaEncodingKey>(curJoint);
      jointFlexion[i][k] = inputMap.find(joint) != inputMap.end() ? (std::stof(inputMap.at(joint)) / maxAnalogValue_) : flexion[i];
      curJoint++;
    }
  }

  // splay is -1.0f -> 1.0f inclusive
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinSplayThumb) != inputMap.end())
    splay[0] = (std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinSplayThumb)) / maxAnalogValue_ - 0.5f) * 2.0f;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinSplayIndex) != inputMap.end())
    splay[1] = (std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinSplayIndex)) / maxAnalogValue_ - 0.5f) * 2.0f;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinSplayMiddle) != inputMap.end())
    splay[2] = (std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinSplayMiddle)) / maxAnalogValue_ - 0.5f) * 2.0f;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinSplayRing) != inputMap.end())
    splay[3] = (std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinSplayRing)) / maxAnalogValue_ - 0.5f) * 2.0f;
  if (inputMap.find(VRCommDataAlphaEncodingKey::FinSplayPinky) != inputMap.end())
    splay[4] = (std::stof(inputMap.at(VRCommDataAlphaEncodingKey::FinSplayPinky)) / maxAnalogValue_ - 0.5f) * 2.0f;

  float joyX = 0;
  float joyY = 0;

  // joystick axis are -1.0f -> 1.0f inclusive
  if (inputMap.find(VRCommDataAlphaEncodingKey::JoyX) != inputMap.end())
    joyX = 2 * std::stof(inputMap.at(VRCommDataAlphaEncodingKey::JoyX)) / maxAnalogValue_ - 1;
  if (inputMap.find(VRCommDataAlphaEncodingKey::JoyY) != inputMap.end())
    joyY = 2 * std::stof(inputMap.at(VRCommDataAlphaEncodingKey::JoyY)) / maxAnalogValue_ - 1;

  VRInputData inputData(
      jointFlexion,
      splay,
      joyX,
      joyY,
      inputMap.find(VRCommDataAlphaEncodingKey::JoyBtn) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::BtnTrg) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::BtnA) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::BtnB) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::GesGrab) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::GesPinch) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::BtnMenu) != inputMap.end(),
      inputMap.find(VRCommDataAlphaEncodingKey::BtnCalib) != inputMap.end());
  return inputData;
}

std::string AlphaEncodingManager::Encode(const VRFFBData& input) {
  std::string result = StringFormat(
      "%s%d%s%d%s%d%s%d%s%d\n",
      VRCommDataAlphaEncodingOutputKeyString.at(VRCommDataAlphaEncodingKey::FinThumb).c_str(),
      input.thumbCurl,
      VRCommDataAlphaEncodingOutputKeyString.at(VRCommDataAlphaEncodingKey::FinIndex).c_str(),
      input.indexCurl,
      VRCommDataAlphaEncodingOutputKeyString.at(VRCommDataAlphaEncodingKey::FinMiddle).c_str(),
      input.middleCurl,
      VRCommDataAlphaEncodingOutputKeyString.at(VRCommDataAlphaEncodingKey::FinRing).c_str(),
      input.ringCurl,
      VRCommDataAlphaEncodingOutputKeyString.at(VRCommDataAlphaEncodingKey::FinPinky).c_str(),
      input.pinkyCurl);

  return result;
}