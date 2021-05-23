#pragma once
#include <functional>
#include <memory>

#include "Encode/EncodingManager.h"

class ICommunicationManager {
 public:
  virtual void Connect() = 0;
  virtual void BeginListener(const std::function<void(VRCommData_t)>& callback) = 0;
  virtual void QueueSend(const std::string& data) = 0;
  virtual bool IsConnected() = 0;
  virtual void Disconnect() = 0;

 private:
  virtual bool Write() = 0;
  std::unique_ptr<IEncodingManager> m_encodingManager;
};