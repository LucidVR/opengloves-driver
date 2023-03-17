// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include "opengloves_interface.h"

class IEncodingService {
 public:
  virtual og::Input DecodePacket(const std::string& buff) = 0;
  virtual std::string EncodePacket(const og::Output& output) = 0;

  virtual ~IEncodingService() = default;
};