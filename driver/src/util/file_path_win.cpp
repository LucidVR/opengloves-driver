// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#ifdef _WIN32

#include <Windows.h>
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

#include "driver_log.h"
#include "file_path.h"
#include "win_util.h"

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