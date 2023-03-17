// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <memory>

#include "crow.h"

class CrowDriverLog : public crow::ILogHandler {
 public:
};

class DriverServerLog {
 public:
  static DriverServerLog& GetInstance() {
    static DriverServerLog instance;

    return instance;
  };

  ~DriverServerLog();

 private:
  DriverServerLog();

 public:
  DriverServerLog(const DriverServerLog&) = delete;
  DriverServerLog& operator=(const DriverServerLog&) = delete;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};