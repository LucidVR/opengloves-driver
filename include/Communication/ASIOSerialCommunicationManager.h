#pragma once
#ifndef _ASIOSERIAL_H
#define _ASIOSERIAL_H

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <thread>

#include <asio.hpp>

class ASIOSerialCommunicationManager : public ICommunicationManager {
public:
  ASIOSerialCommunicationManager(
      const VRSerialConfiguration_t &configuration,
      std::unique_ptr<IEncodingManager> encodingManager);

  bool Connect();
  void BeginListener(const std::function<void(VRCommData_t)> &callback);
  bool IsConnected();
  void Disconnect();

private:
  void ListenerThread(const std::function<void(VRCommData_t)> &callback);
  bool ReceiveNextPacket(std::string& result);

  std::atomic<bool> m_active;
  std::thread m_serialThread;

  VRSerialConfiguration_t m_serialConfiguration;

  std::unique_ptr<IEncodingManager> m_encodingManager;

  asio::io_service m_io;
  asio::serial_port m_serialPort;
};

#endif