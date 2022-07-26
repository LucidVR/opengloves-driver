#pragma once

#include "opengloves_interface.h"

class IEncodingService {
 public:
  virtual og::Input DecodePacket(const std::string& buff) = 0;
  virtual std::string EncodePacket(const og::Output& output) = 0;
};