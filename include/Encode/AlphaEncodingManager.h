#pragma once

#include "DeviceConfiguration.h"
#include "Encode/EncodingManager.h"

#define ALPHA_ENCODING_SETTINGS_SECTION_WITHOUT_PREFIX "encoding_alpha"
#define ALPHA_ENCODING_SETTINGS_SECTION (OPENGLOVES_SECTION_PREFIX ALPHA_ENCODING_SETTINGS_SECTION_WITHOUT_PREFIX)

class AlphaEncodingManager : public IEncodingManager {
public:
	AlphaEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
	 std::string Encode(const VRFFBData_t& input);

	float m_maxAnalogValue;
};