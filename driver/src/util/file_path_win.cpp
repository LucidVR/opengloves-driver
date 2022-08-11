#ifdef _WIN32

#include <Windows.h>
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#include "driver_log.h"
#include "file_path.h"

static std::string GetLastErrorAsString() {
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

std::string GetDriverBinPath() {
  HMODULE hm = nullptr;
  if (GetModuleHandleExA(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCSTR>(&__ImageBase), &hm) == 0) {
    DriverLog("GetModuleHandle failed, error: %s", GetLastErrorAsString().c_str());
    return "";
  }

  char path[1024];
  if (GetModuleFileNameA(hm, path, sizeof path) == 0) {
    DriverLog("GetModuleFileName failed, error: %s", GetLastErrorAsString().c_str());
    return "";
  }

  std::string spath = path;

  //remove filename
  auto filename = spath.rfind('\\');
  if (filename == std::string::npos) return "";

  spath.erase(filename);

  return spath;
}

std::string GetDriverRootPath() {
  std::string path_string = GetDriverBinPath();
  const std::string unwanted = R"(\bin\win64)";
  return path_string.erase(path_string.find(unwanted), unwanted.length());
}

bool CreateBackgroundProcess(const std::string& path, const std::string& name) {
  STARTUPINFOA si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof si);
  si.cb = sizeof si;
  ZeroMemory(&pi, sizeof pi);

  char* carguments = new char[name.length() + 1];
  strcpy(carguments, name.c_str());

  if (!CreateProcess((path + "\\" + name).c_str(), carguments, nullptr, nullptr, FALSE, 0, nullptr, path.c_str(), &si, &pi)) {
    DriverLog("CreateProcess failed. Error: %s", GetLastErrorAsString().c_str());

    delete[] carguments;
    return false;
  };

  delete[] carguments;

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return true;
}

#endif