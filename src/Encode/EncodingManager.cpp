#include "Encode/EncodingManager.h"

VRFFBData::VRFFBData() : VRFFBData(0, 0, 0, 0, 0) {}

VRFFBData::VRFFBData(short thumbCurl, short indexCurl, short middleCurl, short ringCurl, short pinkyCurl)
    : thumbCurl(thumbCurl), indexCurl(indexCurl), middleCurl(middleCurl), ringCurl(ringCurl), pinkyCurl(pinkyCurl) {}

VRInputData::VRInputData() : VRInputData({0, 0, 0, 0, 0}, 0.0f, 0.0f, false, false, false, false, false, false, false, false) {}

VRInputData::VRInputData(std::array<float, 5> flexion, float joyX, float joyY, bool joyButton, bool trgButton, bool aButton, bool bButton, bool grab, bool pinch,
                             bool menu, bool calibrate)
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
