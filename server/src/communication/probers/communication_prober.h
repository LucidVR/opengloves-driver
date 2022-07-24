#pragma once
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "services/communication_service.h"

class ICommunicationProber {
 public:
  virtual int InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) = 0;

  virtual std::string GetName() = 0;
};

class ProberManager {
 public:
  ProberManager(std::function<void(std::unique_ptr<ICommunicationService> service)> device);

  ~ProberManager();

 private:
  void ProberThread(std::unique_ptr<ICommunicationProber>& prober);

  std::function<void(std::unique_ptr<ICommunicationService> service)> callback_;

  std::vector<std::unique_ptr<ICommunicationProber>> probers_;
  std::vector<std::thread> prober_threads_;

  std::atomic<bool> is_active_;
};