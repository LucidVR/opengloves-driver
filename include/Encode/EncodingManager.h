#pragma once

#include <array>
#include <memory>
#include <string>

#include "DriverLog.h"
#include "openvr_driver.h"

namespace VRInputDataVersion {
  struct v1 {
    const std::array<std::array<float, 4>, 5> flexion;
    const std::array<float, 5> splay = {-2.0f, -2.0f, -2.0f, -2.0f, -2.0f};
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

  struct v2 {
    std::array<std::array<float, 4>, 5> flexion;
    std::array<float, 5> splay;
    float joyX;
    float joyY;
    float trgValue;
    bool joyButton;
    bool trgButton;
    bool aButton;
    bool bButton;
    bool grab;
    bool pinch;
    bool menu;
    bool calibrate;
  };
}  // namespace VRInputDataVersion

struct VRInputData : public VRInputDataVersion::v2 {
  VRInputData operator=(const VRInputDataVersion::v1& other) {
    flexion = other.flexion;
    splay = other.splay;
    joyX = other.joyX;
    joyY = other.joyY;
    joyButton = other.joyButton;
    trgButton = other.trgButton;
    aButton = other.aButton;
    bButton = other.bButton;
    grab = other.grab;
    pinch = other.pinch;
    menu = other.menu;
    calibrate = other.calibrate;
  }
};

template <typename T>
std::map<std::string, T> VRInputDataVersions = {{"v1", VRInputDataVersion::v1}, {"v2", VRInputDataVersion::v2}};

// force feedback
struct VRFFBData {
  VRFFBData() : VRFFBData(0, 0, 0, 0, 0){};
  VRFFBData(short thumbCurl, short indexCurl, short middleCurl, short ringCurl, short pinkyCurl)
      : thumbCurl(thumbCurl), indexCurl(indexCurl), middleCurl(middleCurl), ringCurl(ringCurl), pinkyCurl(pinkyCurl){};

  const short thumbCurl;
  const short indexCurl;
  const short middleCurl;
  const short ringCurl;
  const short pinkyCurl;
};

// vibration
struct VRHapticData {
  VRHapticData(vr::VREvent_HapticVibration_t hapticData)
      : duration(hapticData.fDurationSeconds), frequency(hapticData.fFrequency), amplitude(hapticData.fAmplitude){};
  VRHapticData(float duration, float frequency, float amplitude) : duration(duration), frequency(frequency), amplitude(amplitude){};

  const float duration;
  const float frequency;
  const float amplitude;
};

enum class VROutputDataType { ForceFeedback, Haptic };

typedef union VROutputData {
  VROutputData(const VRHapticData& hapticData) : hapticData(hapticData) {}
  VROutputData(const VRFFBData& ffbData) : ffbData(ffbData) {}

  VRHapticData hapticData;
  VRFFBData ffbData;
} VROutputData;

struct VROutput {
  VROutput(const VRHapticData& hapticData) : type(VROutputDataType::Haptic), data(hapticData){};
  VROutput(const VRFFBData& ffbData) : type(VROutputDataType::ForceFeedback), data(ffbData){};

  VROutputDataType type;
  VROutputData data;
};

class EncodingManager {
 public:
  explicit EncodingManager(float maxAnalogValue) : maxAnalogValue_(maxAnalogValue){};
  virtual VRInputData Decode(const std::string& input) = 0;
  virtual std::string Encode(const VROutput& data) = 0;

 protected:
  float maxAnalogValue_;
};

template <typename... Args>
std::string StringFormat(const std::string& format, Args... args) {
  const int sizeS = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'

  if (sizeS <= 0) {
    DriverLog("Error decoding string");
    return "";
  }

  const auto size = static_cast<size_t>(sizeS);
  const auto buf = std::make_unique<char[]>(size);
  std::snprintf(buf.get(), size, format.c_str(), args...);

  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}
