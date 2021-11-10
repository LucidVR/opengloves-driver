#include "Encode/EncodingManager.h"

VRFFBData::VRFFBData() : VRFFBData(0, 0, 0, 0, 0) {}

VRFFBData::VRFFBData(const short thumbCurl, const short indexCurl, const short middleCurl, const short ringCurl, const short pinkyCurl)
    : thumbCurl(thumbCurl), indexCurl(indexCurl), middleCurl(middleCurl), ringCurl(ringCurl), pinkyCurl(pinkyCurl) {}

VRInputData::VRInputData() : VRInputData({0, 0, 0, 0, 0}, 0.0f, 0.0f, false, false, false, false, false, false, false, false) {}

VRInputData::VRInputData(
    const std::array<float, 5> flexion,
    const float joyX,
    const float joyY,
    const bool joyButton,
    const bool trgButton,
    const bool aButton,
    const bool bButton,
    const bool grab,
    const bool pinch,
    const bool menu,
    const bool calibrate)
    : flexion(flexion),
      joyX(joyX),
      joyY(joyY),
      joyButton(joyButton),
      trgButton(trgButton),
      aButton(aButton),
      bButton(bButton),
      grab(grab),
      pinch(pinch),
      menu(menu),
      calibrate(calibrate) {}

EncodingManager::EncodingManager(const float maxAnalogValue) : maxAnalogValue_(maxAnalogValue) {}
