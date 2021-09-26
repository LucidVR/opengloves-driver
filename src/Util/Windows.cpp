#include "Util/Windows.h"

#include <Windows.h>

#include "DriverLog.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

std::string GetDriverPath() {
  HMODULE hm = NULL;
  if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&__ImageBase, &hm) == 0) {
    DriverLog("GetModuleHandle failed, error: %s", GetLastErrorAsString().c_str());
    return std::string();
  }

  char path[1024];
  if (GetModuleFileNameA(hm, path, sizeof(path)) == 0) {
    DriverLog("GetModuleFileName failed, error: %s", GetLastErrorAsString().c_str());
    return std::string();
  }

  std::string pathString = std::string(path);
  const std::string unwanted = "\\bin\\win64\\";
  return pathString.substr(0, pathString.find_last_of("\\/")).erase(pathString.find(unwanted), unwanted.length());
}

bool CreateBackgroundProcess(std::string path) {
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  bool success = true;
  if (!CreateProcess(path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) success = false;

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success;
}

std::string GetLastErrorAsString() {
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) return std::string();

  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

bool CreateBackgroundProcess(const std::string& path) {
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  bool success = true;
  if (!CreateProcess(path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) success = false;

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return success;
}