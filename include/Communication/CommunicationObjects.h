#pragma once

#include <array>

struct VRFFBData_t {
  VRFFBData_t();
  VRFFBData_t(short thumbCurl, short indexCurl, short middleCurl, short ringCurl, short pinkyCurl);

  const short thumbCurl;
  const short indexCurl;
  const short middleCurl;
  const short ringCurl;
  const short pinkyCurl;
};

struct VRInputData_t {
  VRInputData_t();
  VRInputData_t(std::array<float, 5> flexion, float joyX, float joyY, bool joyButton, bool trgButton, bool aButton, bool bButton, bool grab, bool pinch, bool menu,
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
