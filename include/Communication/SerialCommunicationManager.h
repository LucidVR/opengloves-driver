#pragma once

#include <windows.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"

#define SERIAL_COMMUNICATION_SETTINGS_SECTION_WITHOUT_PREFIX "communication_serial"
#define SERIAL_COMMUNICATION_SETTINGS_SECTION (OPENGLOVES_SECTION_PREFIX SERIAL_COMMUNICATION_SETTINGS_SECTION_WITHOUT_PREFIX)

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

  void BeginListener(const std::function<void(VRCommData_t)>& callback);
  bool IsConnected();
  void Disconnect();

  void QueueSend(const VRFFBData_t& data);

 private:
  bool Connect();
  void ListenerThread(const std::function<void(VRCommData_t)>& callback);
  bool ReceiveNextPacket(std::string& buff);
  bool PurgeBuffer();
  bool Write();
  void WaitAttemptConnection();
  bool DisconnectFromDevice();

  void LogMessage(const char* message);
  void LogError(const char* message);

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