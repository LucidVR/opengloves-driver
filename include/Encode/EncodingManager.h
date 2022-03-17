#pragma once

#include <array>
#include <memory>
#include <string>

#include "DriverLog.h"
#include "openvr_driver.h"

// This namespace contains previous versions of the VRInput struct. These structs are used directly when communicating with named pipes, so it is
// important to maintain compatibility with previous versions as to give time for programs that use this type of communication to switch to newer
// versions.

// If you're looking to extend the input struct, create a new version in this namespace, and set up constructors on how to cast previous versions to
// it. Then set VRInputData (used when communicating over bt or usb) to inherit from this new struct.
// If you want your new struct to be compatible with named pipe communication, add an item in the vector in NamedPipeCommunicationManager.cpp with a
// version name.
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
    bool joyButton;
    bool trgButton;
    bool aButton;
    bool bButton;
    bool grab;
    bool pinch;
    bool menu;
    bool calibrate;

    // new
    float trgValue;
  };
}  // namespace VRInputDataVersion

struct VRInputData : public VRInputDataVersion::v2 {
  VRInputData() : VRInputDataVersion::v2(){};

  VRInputData(const VRInputDataVersion::v1& data) {
    flexion = data.flexion;
    splay = data.splay;
    joyX = data.joyX;
    joyY = data.joyY;
    joyButton = data.joyButton;
    trgButton = data.trgButton;
    trgValue = data.trgButton;
    aButton = data.aButton;
    bButton = data.bButton;
    grab = data.grab;
    pinch = data.pinch;
    menu = data.menu;
    calibrate = data.calibrate;
  }
  VRInputData(const VRInputDataVersion::v2& data) {
    flexion = data.flexion;
    splay = data.splay;
    joyX = data.joyX;
    joyY = data.joyY;
    joyButton = data.joyButton;
    trgButton = data.trgButton;
    trgValue = data.trgButton;
    aButton = data.aButton;
    bButton = data.bButton;
    grab = data.grab;
    pinch = data.pinch;
    menu = data.menu;
    calibrate = data.calibrate;
    trgValue = data.trgValue;
  }
};

// force feedback
struct VRFFBData {
  VRFFBData() : VRFFBData(0, 0, 0, 0, 0){};
  VRFFBData(int16_t thumbCurl, int16_t indexCurl, int16_t middleCurl, int16_t ringCurl, int16_t pinkyCurl)
      : thumbCurl(thumbCurl), indexCurl(indexCurl), middleCurl(middleCurl), ringCurl(ringCurl), pinkyCurl(pinkyCurl){};

  const int16_t thumbCurl;
  const int16_t indexCurl;
  const int16_t middleCurl;
  const int16_t ringCurl;
  const int16_t pinkyCurl;
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
