#pragma once
#include "Encode/EncodingManager.h"
#include <functional>
#include <memory>

class ICommunicationManager {
public:
  virtual bool Connect() = 0;
  virtual void BeginListener(const std::function<void(VRCommData_t)> &callback) = 0;
  virtual bool IsConnected() = 0;
  virtual void Disconnect() = 0;

private:
  std::unique_ptr<IEncodingManager> m_encodingManager;
};