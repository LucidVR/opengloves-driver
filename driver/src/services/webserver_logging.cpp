// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "webserver_logging.h"

#include "util/driver_log.h"

class DriverServerLog::Impl : public crow::ILogHandler {
 public:
  Impl() {
    crow::LogLevel log_level = crow::LogLevel::Warning;
#ifdef _DEBUG
    log_level = crow::LogLevel::Info;
#endif

    crow::logger::setLogLevel(log_level);

    crow::logger::setHandler(this);
  }

  void log(std::string message, crow::LogLevel level) override {
    std::string prefix;

    switch (level) {
      case crow::LogLevel::Debug:
        prefix = "debug";
        break;
      case crow::LogLevel::Info:
        prefix = "info";
        break;
      case crow::LogLevel::Warning:
        prefix = "warning";
        break;
      case crow::LogLevel::Error:
        prefix = "error";
        break;
      case crow::LogLevel::Critical:
        prefix = "critical";
        break;
    }

    DriverLog("driver webserver %s: %s", prefix.c_str(), message.c_str());
  }

 private:
};

DriverServerLog::DriverServerLog() {
  pImpl_ = std::make_unique<DriverServerLog::Impl>();
}

DriverServerLog::~DriverServerLog() = default;