// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#include <memory>
#include <string>
#include <functional>

class DriverExternalServer {
 public:
  static DriverExternalServer& GetInstance() {
    static DriverExternalServer instance;

    return instance;
  };

  void RegisterFunctionCallback(const std::string& path, const std::function<bool(const std::string& body)>& callback);
  void RemoveFunctionCallback(const std::string& path);

  void Stop();

  ~DriverExternalServer();

 private:
  DriverExternalServer();

 public:
  DriverExternalServer(const DriverExternalServer&) = delete;
  DriverExternalServer& operator=(const DriverExternalServer&) = delete;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};