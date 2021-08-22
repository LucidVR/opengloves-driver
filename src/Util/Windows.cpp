#include "Util/Windows.h"
#include <Windows.h>
#include "DriverLog.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

std::string GetLastErrorAsString() {
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return std::string();
  }

  LPSTR messageBuffer = nullptr;

  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

std::string GetDriverPath() {
  char path[MAX_PATH];
  HMODULE hm = NULL;

  if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&__ImageBase, &hm) == 0) {
    DriverLog("GetModuleHandle failed, error: %c", GetLastErrorAsString().c_str());
    return "";
  }

  if (GetModuleFileName(hm, path, sizeof(path)) == 0) {
    DriverLog("GetModuleFileName failed, error: %c", GetLastErrorAsString().c_str());
    return "";
  }

  std::string::size_type pos = std::string(path).find_last_of("\\/");
  return std::string(path).substr(0, pos);
}

