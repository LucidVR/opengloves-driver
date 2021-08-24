#pragma once

#include <array>
#include <string>

#include "Communication/CommunicationObjects.h"

class EncodingManager {
 public:
  EncodingManager(float maxAnalogValue);
  virtual VRInputData_t Decode(std::string input) = 0;
  virtual std::string Encode(const VRFFBData_t& data) = 0;

 protected:
  float m_maxAnalogValue;
};

template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
  if (size_s <= 0) {
    DriverLog("Error decoding string");
    return "";
  }
  auto size = static_cast<size_t>(size_s);
  auto buf = std::make_unique<char[]>(size);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}
