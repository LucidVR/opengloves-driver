#pragma once

#include <string>

#include "openvr_driver.h"

extern void DriverLog(const char* pchFormat, ...);

extern void DebugDriverLog(const char* pchFormat, ...);

extern bool InitDriverLog(vr::IVRDriverLog* pDriverLog);
extern void CleanupDriverLog();
