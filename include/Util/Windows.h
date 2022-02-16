#pragma once

#include <string>

extern std::string GetCurrentDirectoryDLL();
extern std::string GetLastErrorAsString();
extern bool CreateBackgroundProcess(const std::string& path);