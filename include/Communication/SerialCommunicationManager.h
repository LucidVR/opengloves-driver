#pragma once

#include <windows.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"

#define ARDUINO_WAIT_TIME 1000

class SerialCommunicationManager : public ICommunicationManager {
 public:
  SerialCommunicationManager(const VRSerialConfiguration_t& configuration, std::unique_ptr<IEncodingManager> encodingManager)
      : m_serialConfiguration(configuration),
      m_encodingManager(std::move(encodingManager)),
      m_isConnected(false),
      m_hSerial(0),
      m_errors(0)
  {
      //initially no force feedback
    VRFFBData_t data(0, 0, 0, 0, 0);

    m_writeString = m_encodingManager->Encode(data);
  };
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
  bool PurgeBuffer();
  bool Write();

  bool m_isConnected;
  // Serial comm handler
  HANDLE m_hSerial;
  // Connection information
  COMSTAT m_status;
  // Error tracking
  DWORD m_errors;
  std::atomic<bool> m_threadActive;
  std::thread m_serialThread;

  VRSerialConfiguration_t m_serialConfiguration;

  std::unique_ptr<IEncodingManager> m_encodingManager;

  std::mutex m_writeMutex;

  std::string m_writeString;
};