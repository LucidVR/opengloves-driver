#pragma once

#include <memory>

#include "opengloves_interface.h"

class OutputOSCServer {
 public:
  static OutputOSCServer& GetInstance() {
    static OutputOSCServer instance;

    return instance;
  };

  void Send(og::Hand hand, const og::InputPeripheralData& input);

  void Stop();

  ~OutputOSCServer();

 private:
  OutputOSCServer();

 public:
  OutputOSCServer(const OutputOSCServer&) = delete;
  OutputOSCServer& operator=(const OutputOSCServer&) = delete;

 private:
  class Impl;
  std::unique_ptr<Impl> pImpl_;
};