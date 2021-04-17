#pragma once

#include <Encode/EncodingManager.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <sstream>
#include <DriverLog.h>

class LegacyEncodingManager : public IEncodingManager {
public:
	LegacyEncodingManager(float maxAnalogValue) : m_maxAnalogValue(maxAnalogValue){};
	
	//decode the given string into a VRCommData_t
	VRCommData_t Decode(std::string input);
private:
	float m_maxAnalogValue;
};