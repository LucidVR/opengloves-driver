#pragma once

#include <functional>
#include <string>

enum CommunicationServiceEventType {
  kCommunicationEvent_UnexpectedDisconnect,
};

struct CommunicationServiceEvent {
  CommunicationServiceEventType type;
};

/*
Interface for a communication service (ie. bluetooth, serial)
It is expected for a service to connect on construction
*/
class ICommunicationService {
 public:
  virtual bool ReceiveNextPacket(std::string& buff) = 0;
  virtual bool RawWrite(const std::string& buff) = 0;

  virtual bool IsConnected() = 0;

  virtual bool PurgeBuffer() = 0;
};