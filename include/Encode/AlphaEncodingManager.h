#pragma once

#include "Encode/EncodingManager.h"

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