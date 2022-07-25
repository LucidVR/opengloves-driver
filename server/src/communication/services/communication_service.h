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
  virtual void AttachEventHandler(std::function<void(CommunicationServiceEvent event)> callback) = 0;

  virtual int ReceiveNextPacket(std::string& buff) = 0;

  virtual int RawWrite(std::string buff) = 0;

  virtual void PurgeBuffer() = 0;

  virtual int Disconnect() = 0;

 private:
  virtual int Connect() = 0;
};