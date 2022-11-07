#include <string>

//path to where the current shared library file is
std::string GetDriverBinPath();

//path to where the driver is installed
std::string GetDriverRootPath();

bool CreateBackgroundProcess(const std::string& path, const std::string& name);