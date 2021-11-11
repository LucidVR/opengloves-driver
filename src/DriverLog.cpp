//========= Copyright Valve Corporation ============//

#include "DriverLog.h"

#include <cstdarg>
#include <cstdio>

static vr::IVRDriverLog* s_pLogFile = nullptr;

bool InitDriverLog(vr::IVRDriverLog* pDriverLog) {
  if (s_pLogFile) return false;
  s_pLogFile = pDriverLog;
  return s_pLogFile != nullptr;
}

void CleanupDriverLog() {
  s_pLogFile = nullptr;
}

static void DriverLogVarArgs(const char* pMsgFormat, va_list args) {
  char buf[1024];
  vsnprintf(buf, sizeof buf, pMsgFormat, args);

  if (s_pLogFile) s_pLogFile->Log(buf);
}

void DriverLog(const char* pchFormat, ...) {
  va_list args;
  va_start(args, pchFormat);

  DriverLogVarArgs(pchFormat, args);

  va_end(args);
}

void DebugDriverLog(const char* pchFormat, ...) {
#ifdef _DEBUG
  va_list args;
  va_start(args, pchFormat);

  DriverLogVarArgs(pchFormat, args);

  va_end(args);
#endif
}