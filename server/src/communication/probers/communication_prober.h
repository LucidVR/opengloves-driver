#pragma once
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "communication/services/communication_service.h"

#include "opengloves_interface.h"

// A prober for a communication method. There can be multiple probers for one communication service (ie. bluetooth, serial, etc.) but the devices that
// each prober attempt to discover should not be the same.
class ICommunicationProber {
 public:
  // A non-blocking method that should return *new* discovered devices into out_devices.
  // The prober should check the device it is about to give back to ensure that it is not already being used.
  virtual og::CommunicationType InquireDevices(std::vector<std::unique_ptr<ICommunicationService>>& out_devices) = 0;

  // Get a pretty name for the prober
  virtual std::string GetName() = 0;
};