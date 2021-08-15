#pragma once

#include "Encode/EncodingManager.h"

enum class VRCommDataAlphaEncodingCharacter : char {
	FIN_THUMB = 'A',
	FIN_INDEX = 'B',
	FIN_MIDDLE = 'C',
	FIN_RING = 'D',
	FIN_PINKY = 'E',
	JOY_X = 'F',
	JOY_Y = 'G',
	JOY_BTN = 'H',
	BTN_TRG = 'I',
	BTN_A = 'J',
	BTN_B = 'K',
	GES_GRAB = 'L',
	GES_PINCH = 'M',
	BTN_MENU = 'N',
	BTN_CALIB = 'O',
};

const char VRCommDataAlphaEncodingCharacters[] = {
	(char)VRCommDataAlphaEncodingCharacter::FIN_THUMB,
	(char)VRCommDataAlphaEncodingCharacter::FIN_INDEX,
	(char)VRCommDataAlphaEncodingCharacter::FIN_MIDDLE,
	(char)VRCommDataAlphaEncodingCharacter::FIN_RING,
	(char)VRCommDataAlphaEncodingCharacter::FIN_PINKY,
	(char)VRCommDataAlphaEncodingCharacter::JOY_X,
	(char)VRCommDataAlphaEncodingCharacter::JOY_Y,
	(char)VRCommDataAlphaEncodingCharacter::JOY_BTN,
	(char)VRCommDataAlphaEncodingCharacter::BTN_TRG,
	(char)VRCommDataAlphaEncodingCharacter::BTN_A,
	(char)VRCommDataAlphaEncodingCharacter::BTN_B,
	(char)VRCommDataAlphaEncodingCharacter::GES_GRAB,
	(char)VRCommDataAlphaEncodingCharacter::GES_PINCH,
	(char)VRCommDataAlphaEncodingCharacter::BTN_MENU,
	(char)VRCommDataAlphaEncodingCharacter::BTN_CALIB,
	(char)0                                             // Turns into a null terminated string
};

class AlphaEncodingManager : public IEncodingManager {
public:
	AlphaEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
    std::string Encode(const VRFFBData_t& input);

       private:
    std::string getArgumentSubstring(std::string str, char del);
           bool argValid(std::string str, char del);

	float m_maxAnalogValue;
};