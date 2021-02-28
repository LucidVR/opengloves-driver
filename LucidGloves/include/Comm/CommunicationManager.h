#pragma once
#include <string>
#include <functional>

/*
	Input Structure:

	fin_thumb
	fin_index
	fin_middle
	fin_ring
	fin_middle
	fin_pinky
	joy_x
	joy_y
	joy_btn
	btn_trg
	btn_a
	btn_b
	ges_grab --
	ges_pinch | -- up for debate if these should be done arduino/driver side, but probably arduino side
*/
struct VRCommData_t
{
	float flexion[5];
	float splay[5];
	float joyX;
	float joyY;	
	bool joyClick;
	bool trgButton;
	bool aButton;
	bool bButton;
	bool grab;
	bool pinch;
};

enum VRCommDataInputPosition {
	FIN_INDEX = 0,
	FIN_MIDDLE = 1,
	FIN_RING = 2,
	FIN_PINKY = 3,
	JOY_X = 4,
	JOY_Y = 5,
	BTN_TRG = 6,
	BTN_A = 7,
	BTN_B = 8,
	GES_GRAB = 9,
	GES_PINCH = 10,
};

class ICommunicationManager {
public:
	virtual void Connect() = 0;
	virtual void BeginListener(const std::function<void(VRCommData_t)>& callback) = 0;
	virtual bool IsConnected() = 0;
	virtual void Disconnect() = 0;
	virtual ~ICommunicationManager() {};
private:
	bool is_connected_;
};