#include "Util/Windows.h"

#include <Windows.h>

#include "DriverLog.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

std::string GetDriverPath() {
  HMODULE hm = nullptr;
  if (GetModuleHandleExA(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(&__ImageBase), &hm) == 0) {
    DriverLog("GetModuleHandle failed, error: %s", GetLastErrorAsString().c_str());
    return std::string();
  }

  char path[1024];
  if (GetModuleFileNameA(hm, path, sizeof path) == 0) {
    DriverLog("GetModuleFileName failed, error: %s", GetLastErrorAsString().c_str());
    return std::string();
  }

  auto pathString = std::string(path);
  const std::string unwanted = R"(\bin\win64\)";
  return pathString.substr(0, pathString.find_last_of("\\/")).erase(pathString.find(unwanted), unwanted.length());
}

bool CreateBackgroundProcess(const std::string& path) {
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof si);
  si.cb = sizeof si;
  ZeroMemory(&pi, sizeof pi);

  bool success = true;
  if (!CreateProcess(path.c_str(), nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) success = false;

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success;
}

std::string GetLastErrorAsString() {
  const DWORD errorMessageId = ::GetLastError();
  if (errorMessageId == 0) return std::string();

  LPSTR messageBuffer = nullptr;
  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      errorMessageId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&messageBuffer),
      0,
      nullptr);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}
