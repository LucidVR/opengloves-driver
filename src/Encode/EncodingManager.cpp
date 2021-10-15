#include "Encode/EncodingManager.h"

VRFFBData_t::VRFFBData_t() : VRFFBData_t(0, 0, 0, 0, 0) {}

VRFFBData_t::VRFFBData_t(short thumbCurl, short indexCurl, short middleCurl, short ringCurl, short pinkyCurl)
    : thumbCurl(thumbCurl), indexCurl(indexCurl), middleCurl(middleCurl), ringCurl(ringCurl), pinkyCurl(pinkyCurl) {}

VRInputData_t::VRInputData_t()
    : VRInputData_t({0, 0, 0, 0, 0}, 0.0f, 0.0f, false, false, false, false, false, false, false, false) {}

VRInputData_t::VRInputData_t(
    std::array<float, 5> flexion,
    float joyX,
    float joyY,
    bool joyButton,
    bool trgButton,
    bool aButton,
    bool bButton,
    bool grab,
    bool pinch,
    bool menu,
    bool calibrate)
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

EncodingManager::EncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue) {}
