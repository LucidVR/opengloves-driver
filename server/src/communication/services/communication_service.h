#pragma once

#include <functional>
#include <string>

enum CommunicationServiceEventType {
  kCommunicationEvent_UnexpectedDisconnect,
};

struct CommunicationServiceEvent {
  CommunicationServiceEventType type;
};

class ICommunicationService {
 public:
  virtual int Connect() = 0;

  virtual void AttachEventHandler(std::function<void(CommunicationServiceEvent event)> callback) = 0;

  virtual int ReceiveNextPacket(std::string& buff) = 0;

  virtual int Disconnect() = 0;
};