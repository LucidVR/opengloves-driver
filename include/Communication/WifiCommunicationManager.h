#pragma once

#define ASIO_STANDALONE
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <sstream>
#include "DriverLog.h"
#include <stdlib.h>
#include <stdio.h>
#include <Winsock2.h>
#include <Ws2bth.h>
#include <BluetoothAPIs.h>

typedef websocketpp::server<websocketpp::config::asio> server;


class WifiCommunicationManager : public ICommunicationManager {
 public:
  WifiCommunicationManager(const VRBTSerialConfiguration_t& configuration,
                               std::unique_ptr<IEncodingManager> encodingManager);
  // connect to the device using serial
  void Connect();
  // start a thread that listens for updates from the device and calls the callback with data
  void BeginListener(const std::function<void(VRCommData_t)>& callback);
  // returns if connected or not
  bool IsConnected();
  // close the serial port
  void Disconnect();

 private:
  void ListenerThread(const std::function<void(VRCommData_t)>& callback);
  bool ReceiveNextPacket(std::string& buff);
  bool PurgeBuffer();
  bool getPairedEsp32BtAddress();
  bool startupWindowsSocket();
  bool connectToEsp32();
  bool sendMessageToEsp32();

  server m_endpoint;
  bool m_isConnected;
  std::atomic<bool> m_threadActive;
  std::thread m_serialThread;
};