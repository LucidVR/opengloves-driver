#include "Communication/SerialCommunicationManager.h"

#include "DriverLog.h"
#include "Util/Windows.h"

SerialCommunicationManager::SerialCommunicationManager(std::unique_ptr<EncodingManager> encodingManager, const VRSerialConfiguration& configuration)
    : CommunicationManager(std::move(encodingManager)), m_serialConfiguration(configuration), m_isConnected(false), m_hSerial(0) {}

bool SerialCommunicationManager::IsConnected() { return m_isConnected; };

bool SerialCommunicationManager::Connect() {
  // We're not yet connected
  m_isConnected = false;

  // Try to connect to the given port throuh CreateFile
  m_hSerial = CreateFile(m_serialConfiguration.port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (m_hSerial == INVALID_HANDLE_VALUE) {
    LogError("Received error connecting to port");
    return false;
  }

  // If connected we try to set the comm parameters
  DCB dcbSerialParams = {0};
  if (!GetCommState(m_hSerial, &dcbSerialParams)) {
    LogError("Failed to get current port parameters");
    return false;
  }

  // Define serial connection parameters for the Arduino board
  dcbSerialParams.BaudRate = m_serialConfiguration.baudRate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;  // reset upon establishing a connection

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

bool SerialCommunicationManager::DisconnectFromDevice() {
  if (!CloseHandle(m_hSerial)) {
    LogError("Error disconnecting from device");
    return false;
  }

  m_isConnected = false;
  LogMessage("Succesfully disconnected from device");
  return true;
}

bool SerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  if (!SetCommMask(m_hSerial, EV_RXCHAR)) {
    LogError("Error setting comm mask");
    return false;
  }

  DWORD dwCommEvent = 0;
  do {
    if (!WaitCommEvent(m_hSerial, &dwCommEvent, NULL)) {
      LogError("Error waiting for event");
      return false;
    }
  } while (m_threadActive && (dwCommEvent & EV_RXCHAR) != EV_RXCHAR);

  if (!m_threadActive) return false;

  char nextChar = 0;
  do {
    DWORD dwRead = 0;
    if (!ReadFile(m_hSerial, &nextChar, 1, &dwRead, NULL)) {
      LogError("Error reading from file");
      return false;
    }
    if (dwRead <= 0 || nextChar == '\n') continue;

    buff += nextChar;
  } while (nextChar != '\n' || buff.length() < 1);

  return true;
}

bool SerialCommunicationManager::SendMessageToDevice() {
  std::lock_guard<std::mutex> lock(m_writeMutex);

  const char* buf = m_writeString.c_str();
  DWORD bytesSent;
  if (!WriteFile(m_hSerial, buf, (DWORD)m_writeString.length(), &bytesSent, 0) || bytesSent < m_writeString.length()) {
    LogError("Error writing to port");
    return false;
  }

  return true;
}

bool SerialCommunicationManager::PurgeBuffer() { return PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR); }

void SerialCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, m_serialConfiguration.port.c_str(), GetLastErrorAsString().c_str());
}

void SerialCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, m_serialConfiguration.port.c_str());
}
