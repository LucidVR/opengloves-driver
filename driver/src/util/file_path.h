#include <string>

//path to where the current shared library file is
std::string GetDriverBinPath();

//path to where the driver is installed
std::string GetDriverRootPath() {
  std::string path_string = GetDriverBinPath();
  const std::string unwanted = R"(\bin\win64)";
  return path_string.erase(path_string.find(unwanted), unwanted.length());
}

bool CreateBackgroundProcess(const std::string& path, const std::string& name);