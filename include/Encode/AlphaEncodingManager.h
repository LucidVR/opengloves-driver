#pragma once

#include "Encode/EncodingManager.h"

class AlphaEncodingManager : public IEncodingManager {
public:
	AlphaEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
private:
    std::string getArgumentSubstring(std::string str, char del);

	float m_maxAnalogValue;
	const char* alphabet = "ABCDEFGHIJKLM";  // expand as more letters are added to manager
};