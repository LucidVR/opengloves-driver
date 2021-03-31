#pragma once
#include <string>
#include <functional>
#include <array>

struct VRCommData_t {
	VRCommData_t(std::array<float, 5> flexion, std::array<float, 5> splay, float joyX, float joyY, bool joyButton, bool trgButton, bool aButton, bool bButton, bool grab, bool pinch) :
		flexion(flexion),
		splay(splay),
		joyX(joyX),
		joyY(joyY),
		joyButton(joyButton),
		trgButton(trgButton),
		aButton(aButton),
		bButton(bButton),
		grab(grab),
		pinch(pinch) {};

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
};

enum VRCommDataInputPosition {
	FIN_PINKY = 0,
	FIN_RING = 1,
	FIN_MIDDLE = 2,
	FIN_INDEX = 3,
	FIN_THUMB = 4,
	JOY_X = 5,
	JOY_Y = 6,
	JOY_BTN = 7,
	BTN_TRG = 8,
	BTN_A = 9,
	BTN_B = 10,
	GES_GRAB = 11,
	GES_PINCH = 12,
};

class IEncodingManager {
public:
	virtual VRCommData_t Decode(std::string input) = 0;
	virtual ~IEncodingManager() {};
private:
	float m_maxAnalogValue;
};