#pragma once

#include "Encode/EncodingManager.h"

static const char* c_alphaEncodingSettingsSection = "encoding_alpha";

class AlphaEncodingManager : public IEncodingManager {
public:
	AlphaEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
	 std::string Encode(const VRFFBData_t& input);

	float m_maxAnalogValue;
};