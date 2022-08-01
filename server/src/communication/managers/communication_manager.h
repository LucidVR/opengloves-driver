#pragma once

#include <functional>
#include <memory>
#include <thread>
#include <atomic>

#include "encoding/encoding_service.h"
#include "opengloves_interface.h"
#include "services/communication_service.h"

class CommunicationManager {
 public:
  CommunicationManager(std::unique_ptr<ICommunicationService> communication_service, std::unique_ptr<IEncodingService> encoding_service);

  void BeginListener(std::function<void(og::Input)> callback);

  ~CommunicationManager();

 private:
  void CommunicationThread();

  std::atomic<bool> thread_active_;
  std::thread communication_thread_;

  std::string queued_write_string;

  std::function<void(og::Input)> callback_;

  std::unique_ptr<ICommunicationService> communication_service_;
  std::unique_ptr<IEncodingService> encoding_service_;
};