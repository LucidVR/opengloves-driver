#pragma once

#include <Encode/EncodingManager.h>

class LegacyEncodingManager : public IEncodingManager {
public:
	LegacyEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
private:
	float m_maxAnalogValue;
};