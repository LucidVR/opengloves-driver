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

  void RegisterFunctionCallback(const std::string& path, const std::function<void(const std::string& body)>& callback);

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