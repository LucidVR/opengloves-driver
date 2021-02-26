#pragma once
#include <string>
#include <functional>
struct VRCommData_t
{
	float flexion[5];
	float splay[5];
	float joyX;
	float joyY;
	bool aButton;
	bool bButton;
	bool joyClick;
	bool grab;
	bool pinch;
};
class ICommunicationManager {
public:
	virtual void Connect() = 0;
	virtual void BeginListener(const std::function<void(const float*)>& callback) = 0;
	virtual bool IsConnected() = 0;
	virtual void Disconnect() = 0;
	virtual ~ICommunicationManager() {};
private:
	bool is_connected_;
};



enum VRCommDataInputPosition {
	FIN_INDEX = 0,
	FIN_MIDDLE = 1,
	FIN_RING = 2,
	FIN_PINKY = 3,
	JOY_X = 4,
	JOY_Y = 5,
	BTN_A = 6,
	BTN_B = 7,
	GES_GRAB = 8,
	GES_PINCH = 9,
};