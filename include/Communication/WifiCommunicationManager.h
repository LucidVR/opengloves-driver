#pragma once

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DriverLog.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

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

  bool m_isConnected;
  std::atomic<bool> m_threadActive;
};