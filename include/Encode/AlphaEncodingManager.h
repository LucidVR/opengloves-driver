#pragma once

#include <Encode/EncodingManager.h>

const char* alphabet = "ABCDE"; //expand as more letters are added to manager

class AlphaEncodingManager : public IEncodingManager {
public:
	AlphaEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
private:
	float m_maxAnalogValue;
};