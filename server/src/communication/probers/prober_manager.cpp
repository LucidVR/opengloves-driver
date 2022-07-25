#include <chrono>

#include "communication_prober.h"
#include "opengloves_interface.h"
#include "probers/serial/prober_serial.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

#define PROBER_MANAGER_PROBE_INTERVAL 2000

ProberManager::ProberManager(std::function<void(std::unique_ptr<ICommunicationService>)> callback) {
  callback_ = callback;

  // place any new probers here
  probers_.emplace_back(std::make_unique<SerialCommunicationProber>());

  is_active_ = true;

  for (auto& prober : probers_) {
    prober_threads_.emplace_back(&ProberManager::ProberThread, prober);

    logger.Log(kLoggerLevel_Info, "Started prober: %s", prober->GetName().c_str());
  }
}

void ProberManager::ProberThread(std::unique_ptr<ICommunicationProber>& prober) {
  while (is_active_) {
    std::vector<std::unique_ptr<ICommunicationService>> services_found;
    prober->InquireDevices(services_found);

    for (auto& service : services_found) {
      logger.Log(kLoggerLevel_Info, "Device discovered on %s with prober: %s", service->GetAddress().c_str(), prober->GetName().c_str());

      callback_(std::move(service));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(PROBER_MANAGER_PROBE_INTERVAL));
  }
}

ProberManager::~ProberManager() {
  if (is_active_.exchange(false)) {
    for (auto& prober_thread : prober_threads_) {
      prober_thread.join();
    }

    for (unsigned int i = 0; i < prober_threads_.size(); i++) {
      const std::string prober_name = probers_[i]->GetName();
      logger.Log(kLoggerLevel_Info, "Attempting to clean up prober: %s", prober_name.c_str());

      prober_threads_[i].join();
      probers_[i].reset();

      logger.Log(kLoggerLevel_Info, "Cleaned up prober: %s", prober_name.c_str());
    }
  }
}