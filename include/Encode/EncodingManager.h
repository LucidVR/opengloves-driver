#pragma once

#include <array>
#include <string>

struct VRFFBData {
  VRFFBData();
  VRFFBData(short thumbCurl, short indexCurl, short middleCurl, short ringCurl, short pinkyCurl);

  const short thumbCurl;
  const short indexCurl;
  const short middleCurl;
  const short ringCurl;
  const short pinkyCurl;
};

struct VRInputData {
  VRInputData();
  VRInputData(std::array<float, 5> flexion, float joyX, float joyY, bool joyButton, bool trgButton, bool aButton, bool bButton, bool grab, bool pinch, bool menu,
                bool calibrate);

  const std::array<float, 5> flexion;
  const float joyX;
  const float joyY;
  const bool joyButton;
  const bool trgButton;
  const bool aButton;
  const bool bButton;
  const bool grab;
  const bool pinch;
  const bool menu;
  const bool calibrate;
};

class EncodingManager {
 public:
  EncodingManager(float maxAnalogValue);
  virtual VRInputData Decode(std::string input) = 0;
  virtual std::string Encode(const VRFFBData& data) = 0;

 protected:
  float m_maxAnalogValue;
};

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
