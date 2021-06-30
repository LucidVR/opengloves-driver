#pragma once
#include <array>
#include <string>

struct VRCommData_t {
    VRCommData_t(std::array<float, 5> flexion, std::array<float, 5> splay, float joyX, float joyY, bool joyButton, bool trgButton, bool aButton, bool bButton, bool grab, bool pinch, bool menu) :
        flexion(flexion),
        splay(splay),
        joyX(joyX),
        joyY(joyY),
        joyButton(joyButton),
        trgButton(trgButton),
        aButton(aButton),
        bButton(bButton),
        grab(grab),
        pinch(pinch),
        menu(menu) {};

    std::array<float, 5> flexion;
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
};

enum VRCommDataInputPosition {
    FIN_PINKY,
    FIN_RING,
    FIN_MIDDLE,
    FIN_INDEX,
    FIN_THUMB,
    JOY_X,
    JOY_Y,
    JOY_BTN,
    BTN_TRG,
    BTN_A,
    BTN_B,
    GES_GRAB,
    GES_PINCH,
    MAX,
};

class IEncodingManager {
public:
    virtual VRCommData_t Decode(std::string input) = 0;
    virtual ~IEncodingManager() {};
private:
    float m_maxAnalogValue;
};