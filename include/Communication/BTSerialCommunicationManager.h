#pragma once
#include <Winsock2.h>
#include <Ws2bth.h>
#include <bluetoothapis.h>
#include <windows.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"
#include "DriverLog.h"

class BTSerialCommunicationManager : public ICommunicationManager {
 public:
  BTSerialCommunicationManager(const VRBTSerialConfiguration_t& configuration, std::unique_ptr<IEncodingManager> encodingManager);

  // start a thread that listens for updates from the device and calls the callback with data
  void BeginListener(const std::function<void(VRCommData_t)>& callback);
  // returns if connected or not
  bool IsConnected();
  // close the serial port
  void Disconnect();

  void QueueSend(const VRFFBData_t& data);

 private:
  bool Connect();
  void ListenerThread(const std::function<void(VRCommData_t)>& callback);
  bool ReceiveNextPacket(std::string& buff);
  bool GetPairedDeviceBtAddress();
  bool StartupWindowsSocket();
  bool ConnectToDevice();
  bool SendMessageToDevice();
  void WaitAttemptConnection();
  bool DisconnectFromDevice();
  void LogError(const char* message);
  void LogMessage(const char* message);

  std::atomic<bool> m_isConnected;
  std::atomic<bool> m_threadActive;
  std::thread m_serialThread;

  std::unique_ptr<IEncodingManager> m_encodingManager;

  VRBTSerialConfiguration_t m_btSerialConfiguration;

  BTH_ADDR m_deviceBtAddress;
  SOCKADDR_BTH m_btSocketAddress;
  SOCKET m_btClientSocket;
  WCHAR* m_wcDeviceName;

  std::mutex m_writeMutex;

  std::string m_writeString = "\n";
};