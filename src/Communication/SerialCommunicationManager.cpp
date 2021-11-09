#include "Communication/SerialCommunicationManager.h"

#include <utility>

#include "DriverLog.h"
#include "Util/Windows.h"

SerialCommunicationManager::SerialCommunicationManager(
    std::unique_ptr<EncodingManager> encodingManager, VRSerialConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration)
    : CommunicationManager(std::move(encodingManager), deviceConfiguration),
      _serialConfiguration(std::move(configuration)),
      _isConnected(false),
      _hSerial(nullptr) {}

bool SerialCommunicationManager::IsConnected() {
  return _isConnected;
};

bool SerialCommunicationManager::Connect() {
  LogMessage("Attempting connection to device");
  // We're not yet connected
  _isConnected = false;

  // Try to connect to the given port throuh CreateFile
  _hSerial = CreateFile(_serialConfiguration.port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (_hSerial == INVALID_HANDLE_VALUE) {
    LogError("Received error connecting to port");
    return false;
  }

  // If connected we try to set the comm parameters
  DCB dcbSerialParams = {0};
  if (!GetCommState(_hSerial, &dcbSerialParams)) {
    LogError("Failed to get current port parameters");
    return false;
  }

  // Define serial connection parameters for the Arduino board
  dcbSerialParams.BaudRate = _serialConfiguration.baudRate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;  // reset upon establishing a connection

  // set the parameters and check for their proper application
  if (!SetCommState(_hSerial, &dcbSerialParams)) {
    LogError("Failed to set serial parameters");
    return false;
  }

  // If everything went fine we're connected
  _isConnected = true;

  PurgeBuffer();

  LogMessage("Successfully connected to device");

  return true;
}

bool SerialCommunicationManager::DisconnectFromDevice() {
  if (!CloseHandle(_hSerial)) {
    LogError("Error disconnecting from device");
    return false;
  }

  _isConnected = false;
  LogMessage("Successfully disconnected from device");
  return true;
}

bool SerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  if (!SetCommMask(_hSerial, EV_RXCHAR)) {
    LogError("Error setting comm mask");
    return false;
  }

  DWORD dwCommEvent = 0;
  do {
    if (!WaitCommEvent(_hSerial, &dwCommEvent, nullptr)) {
      LogError("Error waiting for event");
      return false;
    }
  } while (_threadActive && (dwCommEvent & EV_RXCHAR) != EV_RXCHAR);

  if (!_threadActive) return false;

  char nextChar = 0;
  do {
    DWORD dwRead = 0;
    if (!ReadFile(_hSerial, &nextChar, 1, &dwRead, nullptr)) {
      LogError("Error reading from file");
      return false;
    }
    if (dwRead <= 0 || nextChar == '\n') continue;

    buff += nextChar;
  } while ((nextChar != '\n' || buff.length() < 1) && _threadActive);

  // If the glove firmware sends data more often than we poll for it then the buffer
  // will become saturated and block future reads. We've got the data we need so purge
  // anything else left in the buffer. There should be more data ready for us in the
  // buffer by the next time we poll for it.
  PurgeBuffer();

  return true;
}

bool SerialCommunicationManager::SendMessageToDevice() {
  std::lock_guard lock(_writeMutex);

  const char* buf = _writeString.c_str();
  DWORD bytesSent;
  if (!WriteFile(_hSerial, buf, static_cast<DWORD>(_writeString.length()), &bytesSent, nullptr) || bytesSent < _writeString.length()) {
    LogError("Error writing to port");
    return false;
  }

  return true;
}

bool SerialCommunicationManager::PurgeBuffer() const {
  return PurgeComm(_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, _serialConfiguration.port.c_str(), GetLastErrorAsString().c_str());
}

void SerialCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, _serialConfiguration.port.c_str());
}
