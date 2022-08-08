#include "encoding/alpha_encoding/alpha_encoding_service.h"

#include <algorithm>
#include <map>
#include <stdexcept>

static og::Logger& logger = og::Logger::GetInstance();

static const int max_analog_value = 4095;

static const std::map<std::string, AlphaEncodingKey> alpha_encoding_input_key_strings{
    {"A", kAlphaEncodingKey_ThumbCurl},           // whole thumb curl
    {"(AB)", kAlphaEncodingKey_ThumbSplay},       // whole thumb splay
    {"B", kAlphaEncodingKey_IndexCurl},           // whole index curl
    {"(BB)", kAlphaEncodingKey_IndexSplay},       // whole index splay
    {"C", kAlphaEncodingKey_MiddleCurl},          // whole middle curl
    {"(CB)", kAlphaEncodingKey_MiddleSplay},      // whole middle splay
    {"D", kAlphaEncodingKey_RingCurl},            // whole ring curl
    {"(DB)", kAlphaEncodingKey_RingSplay},        // whole ring splay
    {"E", kAlphaEncodingKey_PinkyCurl},           // whole pinky curl
    {"(EB)", kAlphaEncodingKey_PinkySplay},       // whole pinky splay
    {"(AAA)", kAlphaEncodingKey_ThumbJoint0},     // thumb joint 0
    {"(AAB)", kAlphaEncodingKey_ThumbJoint1},     // thumb joint 1
    {"(AAC)", kAlphaEncodingKey_ThumbJoint2},     // thumb joint 2
    {"(AAD)", kAlphaEncodingKey_ThumbJoint3},     // thumb joint 3 (not used)
    {"(BAA)", kAlphaEncodingKey_IndexJoint0},     // index joint 0
    {"(BAB)", kAlphaEncodingKey_IndexJoint1},     // index joint 1
    {"(BAC)", kAlphaEncodingKey_IndexJoint2},     // index joint 2
    {"(BAD)", kAlphaEncodingKey_IndexJoint3},     // index joint 3
    {"(CAA)", kAlphaEncodingKey_MiddleJoint0},    // middle joint 0
    {"(CAB)", kAlphaEncodingKey_MiddleJoint1},    // middle joint 1
    {"(CAC)", kAlphaEncodingKey_MiddleJoint2},    // middle joint 2
    {"(CAD)", kAlphaEncodingKey_MiddleJoint3},    // middle joint 3
    {"(DAA)", kAlphaEncodingKey_RingJoint0},      // ring joint 0
    {"(DAB)", kAlphaEncodingKey_RingJoint1},      // ring joint 1
    {"(DAC)", kAlphaEncodingKey_RingJoint2},      // ring joint 2
    {"(DAD)", kAlphaEncodingKey_RingJoint3},      // ring joint 3
    {"(EAA)", kAlphaEncodingKey_PinkyJoint0},     // pinky joint 0
    {"(EAB)", kAlphaEncodingKey_PinkyJoint1},     // pinky joint 1
    {"(EAC)", kAlphaEncodingKey_PinkyJoint2},     // pinky joint 2
    {"(EAD)", kAlphaEncodingKey_PinkyJoint3},     // pinky joint 3
    {"F", kAlphaEncodingKey_MainJoystick_X},      // main joystick x component
    {"G", kAlphaEncodingKey_MainJoystick_Y},      // main joystick y component
    {"H", kAlphaEncodingKey_MainJoystick_Click},  // main joystick button
    {"I", kAlphaEncodingKey_Trigger_Click},       // trigger button
    {"J", kAlphaEncodingKey_A_Click},             // A button
    {"K", kAlphaEncodingKey_B_Click},             // B button
    {"L", kAlphaEncodingKey_Grab_Gesture},        // grab gesture (boolean)
    {"M", kAlphaEncodingKey_Pinch_Gesture},       // pinch gesture (boolean)
    {"N", kAlphaEncodingKey_Menu_Click},          // system button pressed (opens SteamVR menu)
    {"O", kAlphaEncodingKey_Calibration_Click},   // calibration button
    {"P", kAlphaEncodingKey_Trigger_Value},       // analog trigger value

    {"Z", kAlphaEncodingKey_Info},
    {"(ZV)", kAlphaEncodingKey_Info_FWVersion},   // firmware version
    {"(ZG)", kAlphaEncodingKey_Info_DeviceType},  // glove type (ie lucidgloves)
    {"(ZH)", kAlphaEncodingKey_Info_Hand},        // hand (left/right)

    {"", kAlphaEncodingKey_Max}  // Junk key
};

static const std::map<AlphaEncodingKey, std::string> alpha_encoding_output_key_strings{
    {kAlphaEncodingKey_Info, "Z"},

    {kAlphaEncodingKey_Info_StartStreaming, "(ZA)"},
    {kAlphaEncodingKey_Info_StopStreaming, "(ZZ)"},

    {kAlphaEncodingKey_ThumbCurl, "A"},   // thumb force feedback
    {kAlphaEncodingKey_IndexCurl, "B"},   // index force feedback
    {kAlphaEncodingKey_MiddleCurl, "C"},  // middle force feedback
    {kAlphaEncodingKey_RingCurl, "D"},    // ring force feedback
    {kAlphaEncodingKey_PinkyCurl, "E"},   // pinky force feedback

    {kAlphaEncodingKey_OutHapticFrequency, "F"},  // haptic vibration frequency
    {kAlphaEncodingKey_OutHapticDuration, "G"},   // haptic vibration duration
    {kAlphaEncodingKey_OutHapticAmplitude, "H"},  // haptic vibration amplitude
};

static const std::string alpha_encoding_key_characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ()";

static bool CharacterIsValid(const std::string& buff, size_t index) {
  return index < buff.length() && buff[index] < 255 && buff[index] >= 0;
}

static bool CharacterIsKeyCharacter(const char character) {
  return alpha_encoding_key_characters.find(character) != std::string::npos;
}

static inline bool KeyInMap(AlphaEncodingKey key, const std::map<AlphaEncodingKey, std::string>& map) {
  return map.find(key) != map.end();
}

static std::map<AlphaEncodingKey, std::string> ParseToMap(const std::string& buff) {
  std::map<AlphaEncodingKey, std::string> result;

  size_t i = 0;
  while (i < buff.length()) {
    std::string current_key = "";

    // parse a key
    if (buff[i] == '(') {
      while (CharacterIsValid(buff, i) && CharacterIsKeyCharacter(buff[i])) {
        current_key += buff[i];
        i++;
      }
    } else if (CharacterIsKeyCharacter(buff[i])) {
      current_key = buff[i];
      i++;
    } else {
      i++;
      continue;
    }

    // we have a valid key
    std::string current_value = "";
    while (CharacterIsValid(buff, i) && isdigit(buff[i])) {
      current_value += buff[i];

      i++;
    }

    if (alpha_encoding_input_key_strings.find(current_key) != alpha_encoding_input_key_strings.end()) {
      result.insert_or_assign(alpha_encoding_input_key_strings.at(current_key), current_value);
    } else {
      logger.Log(og::kLoggerLevel_Warning, "Unable to insert key as it was not found in map: %s", current_key.c_str());
    }
  }

  return result;
}

AlphaEncodingService::AlphaEncodingService(const og::EncodingConfiguration& encoding_configuration) {
  configuration_ = encoding_configuration;
}

og::InputPeripheralData AlphaEncodingService::DecodePeripheralPacket(const std::map<AlphaEncodingKey, std::string>& input_map) {
  og::InputPeripheralData result;

  // default flexion curl values for the whole finger
  std::array<float, 5> flexion;
  flexion.fill(-1.0f);

  // parse full finger curls first in a temporary array (so we can fallback to them if needed)
  if (KeyInMap(kAlphaEncodingKey_ThumbCurl, input_map))
    flexion[0] = std::stof(input_map.at(kAlphaEncodingKey_ThumbCurl)) / configuration_.max_analog_value;
  if (KeyInMap(kAlphaEncodingKey_IndexCurl, input_map))
    flexion[1] = std::stof(input_map.at(kAlphaEncodingKey_IndexCurl)) / configuration_.max_analog_value;
  if (KeyInMap(kAlphaEncodingKey_MiddleCurl, input_map))
    flexion[2] = std::stof(input_map.at(kAlphaEncodingKey_MiddleCurl)) / configuration_.max_analog_value;
  if (KeyInMap(kAlphaEncodingKey_RingCurl, input_map))
    flexion[3] = std::stof(input_map.at(kAlphaEncodingKey_RingCurl)) / configuration_.max_analog_value;
  if (KeyInMap(kAlphaEncodingKey_PinkyCurl, input_map))
    flexion[4] = std::stof(input_map.at(kAlphaEncodingKey_PinkyCurl)) / configuration_.max_analog_value;

  int current_joint = kAlphaEncodingKey_ThumbJoint0;

  // fill individual joint curls
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 4; j++) {
      AlphaEncodingKey joint = static_cast<AlphaEncodingKey>(current_joint);
      result.flexion[i][j] = input_map.find(joint) != input_map.end() ? std::stof(input_map.at(joint)) / configuration_.max_analog_value : flexion[i];

      current_joint++;
    }
  }

  // splay: -1.0f -> 1.0f inclusive
  if (KeyInMap(kAlphaEncodingKey_ThumbSplay, input_map))
    result.splay[0] = (std::stof(input_map.at(kAlphaEncodingKey_ThumbSplay)) / configuration_.max_analog_value - 0.5f) * 2.0f;
  if (KeyInMap(kAlphaEncodingKey_IndexSplay, input_map))
    result.splay[1] = (std::stof(input_map.at(kAlphaEncodingKey_IndexSplay)) / configuration_.max_analog_value - 0.5f) * 2.0f;
  if (KeyInMap(kAlphaEncodingKey_MiddleSplay, input_map))
    result.splay[2] = (std::stof(input_map.at(kAlphaEncodingKey_MiddleSplay)) / configuration_.max_analog_value - 0.5f) * 2.0f;
  if (KeyInMap(kAlphaEncodingKey_RingSplay, input_map))
    result.splay[3] = (std::stof(input_map.at(kAlphaEncodingKey_RingSplay)) / configuration_.max_analog_value - 0.5f) * 2.0f;
  if (KeyInMap(kAlphaEncodingKey_PinkySplay, input_map))
    result.splay[4] = (std::stof(input_map.at(kAlphaEncodingKey_PinkySplay)) / configuration_.max_analog_value - 0.5f) * 2.0f;

  // main joystick
  if (KeyInMap(kAlphaEncodingKey_MainJoystick_X, input_map))
    result.joystick.x = 2 * std::stof(input_map.at(kAlphaEncodingKey_MainJoystick_X)) / configuration_.max_analog_value - 1;
  if (KeyInMap(kAlphaEncodingKey_MainJoystick_Y, input_map))
    result.joystick.y = 2 * std::stof(input_map.at(kAlphaEncodingKey_MainJoystick_Y)) / configuration_.max_analog_value - 1;

  // trigger value: 0.0f -> 1.0f inclusive
  if (KeyInMap(kAlphaEncodingKey_Trigger_Value, input_map))
    result.trigger.value = std::stof(input_map.at(kAlphaEncodingKey_Trigger_Value)) / configuration_.max_analog_value;

  result.joystick.pressed = KeyInMap(kAlphaEncodingKey_MainJoystick_Click, input_map);
  result.trigger.pressed = KeyInMap(kAlphaEncodingKey_Trigger_Click, input_map);
  result.A.pressed = KeyInMap(kAlphaEncodingKey_A_Click, input_map);
  result.B.pressed = KeyInMap(kAlphaEncodingKey_B_Click, input_map);
  result.grab.activated = KeyInMap(kAlphaEncodingKey_Grab_Gesture, input_map);
  result.pinch.activated = KeyInMap(kAlphaEncodingKey_Pinch_Gesture, input_map);
  result.menu.pressed = KeyInMap(kAlphaEncodingKey_Menu_Click, input_map);
  result.calibrate.pressed = KeyInMap(kAlphaEncodingKey_Calibration_Click, input_map);

  if (result == og::InputPeripheralData{}) throw std::runtime_error("Peripheral packet was empty.");

  return result;
}

og::InputInfoData AlphaEncodingService::DecodeInfoPacket(const std::map<AlphaEncodingKey, std::string>& input_map) {
  og::InputInfoData result{};

  if (KeyInMap(kAlphaEncodingKey_Info_FWVersion, input_map))  // get firmware version
    result.firmware_version = std::stoi(input_map.at(kAlphaEncodingKey_Info_FWVersion));
  if (KeyInMap(kAlphaEncodingKey_Info_DeviceType, input_map))  // device type (ie. lucidgloves)
    result.device_type = (og::DeviceType)std::stoi(input_map.at(kAlphaEncodingKey_Info_DeviceType));
  if (KeyInMap(kAlphaEncodingKey_Info_Hand, input_map))  // handedness (left/right)
    result.hand = (og::Hand)std::stoi(input_map.at(kAlphaEncodingKey_Info_Hand));

  if (result == og::InputInfoData{}) throw std::runtime_error("Info packet was empty.");

  return result;
}

og::Input AlphaEncodingService::DecodePacket(const std::string& buff) {
  og::Input result{};

  try {
    std::map<AlphaEncodingKey, std::string> input_map = ParseToMap(buff);

    // info packet
    if (input_map.contains(kAlphaEncodingKey_Info)) {
      result.type = og::kInputDataType_Info;
      result.data.info = DecodeInfoPacket(input_map);
      return result;
    }

    // else try decode a peripheral packet (packet that encodes
    result.type = og::kInputDataType_Peripheral;
    result.data.peripheral = DecodePeripheralPacket(input_map);
    return result;

  } catch (const std::exception& ex) {
    logger.Log(og::kLoggerLevel_Error, "Failed to parse data with alpha_encoding encoding: %s", ex.what());
    logger.Log(og::kLoggerLevel_Info, "Packet that failed: %s", buff.c_str());

    result.type = og::kInputDataType_Invalid;
    return result;
  }
}

template <typename... Args>
static std::string StringFormat(const std::string& format, Args... args) {
  const int sizeS = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'

  if (sizeS <= 0) {
    logger.Log(og::kLoggerLevel_Error, "Error encoding output");
    return "";
  }

  const auto size = static_cast<size_t>(sizeS);
  const auto buf = std::make_unique<char[]>(size);
  std::snprintf(buf.get(), size, format.c_str(), args...);

  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}

std::string AlphaEncodingService::EncodePacket(const og::Output& output) {
  switch (output.type) {
    case og::kOutputDataType_FetchInfo: {
      const og::OutputFetchInfoData& data = output.data.fetch_info;
      std::string result = "";

      if (data.start_streaming) result += alpha_encoding_output_key_strings.at(kAlphaEncodingKey_Info_StartStreaming);
      if (data.get_info) result += alpha_encoding_output_key_strings.at(kAlphaEncodingKey_Info);

      return result;
    }

    case og::kOutputData_Type_ForceFeedback: {
      const og::OutputForceFeedbackData& data = output.data.force_feedback_data;

      return StringFormat(
          "%s%d%s%d%s%d%s%d%s%d\n",
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_ThumbCurl).c_str(),
          data.thumb,
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_IndexCurl).c_str(),
          data.index,
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_MiddleCurl).c_str(),
          data.middle,
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_RingCurl).c_str(),
          data.ring,
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_PinkyCurl).c_str(),
          data.pinky);
    }

    case og::kOutputDataType_Haptic: {
      const og::OutputHapticData& data = output.data.haptic_data;

      return StringFormat(
          "%s%.2f%s%.2f%s%.2f\n",
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_OutHapticFrequency).c_str(),
          data.frequency,
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_OutHapticDuration).c_str(),
          data.duration,
          alpha_encoding_output_key_strings.at(kAlphaEncodingKey_OutHapticAmplitude).c_str(),
          data.amplitude);
    }

    default:
      logger.Log(og::kLoggerLevel_Warning, "Unable to deduce output data type.");
    case og::kOutputDataType_Empty: {
      return "\n";
    }
  }
}