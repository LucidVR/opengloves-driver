#include <Communication/SerialCommunicationManager.h>

#include <chrono>

#include "DriverLog.h"

static const uint32_t c_listenerWaitTime = 1000;

static std::string GetLastErrorAsString() {
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return std::string();
  }

  LPSTR messageBuffer = nullptr;

  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

bool SerialCommunicationManager::Connect() {
  // We're not yet connected
  m_isConnected = false;

  // Try to connect to the given port throuh CreateFile
  m_hSerial = CreateFile(m_serialConfiguration.port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (this->m_hSerial == INVALID_HANDLE_VALUE) {
    LogError("Received error connecting to port");
    return false;
  }

  // If connected we try to set the comm parameters
  DCB dcbSerialParams = {0};

  if (!GetCommState(m_hSerial, &dcbSerialParams)) {
    LogError("Failed to get current port parameters");
    return false;
  }

  // Define serial connection parameters for the arduino board
  dcbSerialParams.BaudRate = m_serialConfiguration.baudRate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;

  // reset upon establishing a connection
  dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

  // set the parameters and check for their proper application
  if (!SetCommState(m_hSerial, &dcbSerialParams)) {
    LogError("Failed to set serial parameters");
    return false;
  }

  // If everything went fine we're connected
  m_isConnected = true;

  PurgeBuffer();

  return true;
}

void SerialCommunicationManager::WaitAttemptConnection() {
  while (m_threadActive && !IsConnected() && !Connect()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
  }
}

void SerialCommunicationManager::BeginListener(const std::function<void(VRCommData_t)>& callback) {
  m_threadActive = true;
  m_serialThread = std::thread(&SerialCommunicationManager::ListenerThread, this, callback);
}

void SerialCommunicationManager::ListenerThread(const std::function<void(VRCommData_t)>& callback) {
  WaitAttemptConnection();

  while (m_threadActive) {
    std::string receivedString;
    bool readSuccessful = ReceiveNextPacket(receivedString);

    if (readSuccessful) {
        try {
            VRCommData_t commData = m_encodingManager->Decode(receivedString);
            callback(commData);
            Write();
            continue;
        }
        catch (const std::invalid_argument& ia) {
            LogMessage((std::string("Received error from encoding manager: ") + ia.what()).c_str());
        }
    }
    LogMessage("Detected device error. Disconnecting device and attempting reconnection");
    if (DisconnectFromDevice()) {
    WaitAttemptConnection();
    LogMessage("Sucessfully reconnected to device");
    continue;
    }

    LogMessage("Could not connect to device. Closing listener");
    Disconnect();
  }
}

bool SerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  DWORD dwCommEvent = 0;
  DWORD dwRead = 0;

  if (!SetCommMask(m_hSerial, EV_RXCHAR)) {
    LogError("Error setting comm mask");
    return false;
  }

  char nextChar = 0;

  if (!WaitCommEvent(m_hSerial, &dwCommEvent, NULL)) {
    LogError("Error waiting for event");
    return false;
  }

  do {
    if (!ReadFile(m_hSerial, &nextChar, 1, &dwRead, NULL)) {
      LogError("Error reading from file");
      return false;
    }

    buff += nextChar;
  } while (nextChar != '\n');

  return true;
}

void SerialCommunicationManager::QueueSend(const VRFFBData_t& data) {
  std::lock_guard<std::mutex> lock(m_writeMutex);

  m_writeString = m_encodingManager->Encode(data);
}

bool SerialCommunicationManager::Write() {
  std::lock_guard<std::mutex> lock(m_writeMutex);

  const char* buf = m_writeString.c_str();

  DWORD bytesSend;

  if (!WriteFile(this->m_hSerial, (void*)buf, (DWORD)strlen(buf), &bytesSend, 0)) {
    LogError("Error writing to port");
    return false;
  }

  return true;
}

bool SerialCommunicationManager::PurgeBuffer() { return PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR); }

void SerialCommunicationManager::Disconnect() {
  if (m_isConnected) {
    if (m_threadActive) {
      m_threadActive = false;
      m_serialThread.join();
    }
    DisconnectFromDevice();
  }
}

bool SerialCommunicationManager::DisconnectFromDevice() {
  if (!CloseHandle(m_hSerial)) {
    LogError("Error disconnecting from device");
    return false;
  }

  m_isConnected = false;
  LogMessage("Succesfully disconnected from device");
  return true;
}

bool SerialCommunicationManager::IsConnected() { return m_isConnected; };

void SerialCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, m_serialConfiguration.port.c_str(), GetLastErrorAsString().c_str());
}

void SerialCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, m_serialConfiguration.port.c_str());
}