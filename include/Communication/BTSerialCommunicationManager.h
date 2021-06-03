#pragma once
#include <Winsock2.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>
#include "CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DriverLog.h"

#include <windows.h>
#include <bluetoothapis.h>
#include <Ws2bth.h>


#define ARDUINO_WAIT_TIME 1000
class BTSerialCommunicationManager : public ICommunicationManager {
 public:
  BTSerialCommunicationManager(const VRBTSerialConfiguration_t& configuration, std::unique_ptr<IEncodingManager> encodingManager);
  // connect to the device using serial
  void Connect();
  // start a thread that listens for updates from the device and calls the callback with data
  void BeginListener(const std::function<void(VRCommData_t)>& callback);
  // returns if connected or not
  bool IsConnected();
  // close the serial port
  void Disconnect();

  void QueueSend(const VRFFBData_t& data);

 private:
  void ListenerThread(const std::function<void(VRCommData_t)>& callback);
  bool ReceiveNextPacket(std::string& buff);
  bool getPairedEsp32BtAddress();
  bool startupWindowsSocket();
  bool connectToEsp32();
  bool sendMessageToEsp32();
  bool m_isConnected;
  std::atomic<bool> m_threadActive;
  std::thread m_serialThread;

  std::unique_ptr<IEncodingManager> m_encodingManager;

  VRBTSerialConfiguration_t m_btSerialConfiguration;

  BTH_ADDR m_esp32BtAddress;
  SOCKADDR_BTH m_btSocketAddress;
  SOCKET m_btClientSocket;
  WCHAR* m_wcDeviceName;
};