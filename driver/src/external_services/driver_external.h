#pragma once

#include <memory>

class DriverExternalServer {
 public:
  static DriverExternalServer& GetInstance() {
    static DriverExternalServer instance;

    return instance;
  };

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