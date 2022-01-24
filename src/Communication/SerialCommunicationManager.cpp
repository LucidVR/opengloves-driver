#include "Communication/SerialCommunicationManager.h"

#include <utility>

#include "DriverLog.h"
#include "Util/Windows.h"

SerialCommunicationManager::SerialCommunicationManager(
    std::unique_ptr<EncodingManager> encodingManager, VRSerialConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration)
    : CommunicationManager(std::move(encodingManager), deviceConfiguration),
      serialConfiguration_(std::move(configuration)),
      isConnected_(false),
      hSerial_(nullptr) {}

bool SerialCommunicationManager::IsConnected() {
  return isConnected_;
};

bool SerialCommunicationManager::SetCommunicationTimeout(
    unsigned long ReadIntervalTimeout,
    unsigned long ReadTotalTimeoutMultiplier,
    unsigned long ReadTotalTimeoutConstant,
    unsigned long WriteTotalTimeoutMultiplier,
    unsigned long WriteTotalTimeoutConstant) {
  COMMTIMEOUTS timeout;

  timeout.ReadIntervalTimeout = ReadIntervalTimeout;
  timeout.ReadTotalTimeoutConstant = ReadTotalTimeoutConstant;
  timeout.ReadTotalTimeoutMultiplier = ReadTotalTimeoutMultiplier;
  timeout.WriteTotalTimeoutConstant = WriteTotalTimeoutConstant;
  timeout.WriteTotalTimeoutMultiplier = WriteTotalTimeoutMultiplier;

  if (!SetCommTimeouts(hSerial_, &timeout)) return false;

  return true;
}

bool SerialCommunicationManager::Connect() {
  LogMessage("Attempting connection to device");
  // We're not yet connected
  isConnected_ = false;

  // Try to connect to the given port throuh CreateFile
  hSerial_ = CreateFile(serialConfiguration_.port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (hSerial_ == INVALID_HANDLE_VALUE) {
    LogError("Received error connecting to port");
    return false;
  }

  // If connected we try to set the comm parameters
  DCB dcbSerialParams = {0};
  if (!GetCommState(hSerial_, &dcbSerialParams)) {
    LogError("Failed to get current port parameters");
    return false;
  }

  // Define serial connection parameters for the Arduino board
  dcbSerialParams.BaudRate = serialConfiguration_.baudRate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;  // reset upon establishing a connection

  // set the parameters and check for their proper application
  if (!SetCommState(hSerial_, &dcbSerialParams)) {
    LogError("Failed to set serial parameters");
    return false;
  }

  if (!SetCommunicationTimeout(50, 0, 0, 50, 0)) {
    LogError("Failed to set communication timeout");
    return false;
  }

  if (!SetupComm(hSerial_, 200, 200)) {
    LogError("Failed to setup comm");
    return false;
  }

  // If everything went fine we're connected
  isConnected_ = true;

  PurgeBuffer();

  LogMessage("Successfully connected to device");

  return true;
}

bool SerialCommunicationManager::DisconnectFromDevice() {
  if (!CloseHandle(hSerial_)) {
    LogError("Error disconnecting from device");
    return false;
  }

  isConnected_ = false;
  LogMessage("Successfully disconnected from device");
  return true;
}

bool SerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  if (!SetCommMask(hSerial_, EV_RXCHAR)) {
    LogError("Error setting comm mask");
    return false;
  }

  DWORD dwCommEvent = 0;
  do {
    if (!WaitCommEvent(hSerial_, &dwCommEvent, nullptr)) {
      LogError("Error waiting for event");
      return false;
    }
  } while (threadActive_ && (dwCommEvent & EV_RXCHAR) != EV_RXCHAR);

  if (!threadActive_) return false;

  char nextChar = 0;
  do {
    DWORD dwRead = 0;
    if (!ReadFile(hSerial_, &nextChar, 1, &dwRead, nullptr)) {
      LogError("Error reading from file");
      return false;
    }
    if (dwRead <= 0 || nextChar == '\n') continue;

    buff += nextChar;
  } while ((nextChar != '\n' || buff.length() < 1) && threadActive_);

  // If the glove firmware sends data more often than we poll for it then the buffer
  // will become saturated and block future reads. We've got the data we need so purge
  // anything else left in the buffer. There should be more data ready for us in the
  // buffer by the next time we poll for it.
  // TODO: This is currently causing lag on ESP32's so purging has been removed for now.
  // Things to try in the future are purging on a time increment, or shrinking the buffer size.
  // PurgeBuffer();

  return true;
}

bool SerialCommunicationManager::SendMessageToDevice() {
  std::lock_guard lock(writeMutex_);

  const char* buf = writeString_.c_str();
  DWORD bytesSent;
  if (!WriteFile(hSerial_, buf, static_cast<DWORD>(writeString_.length()), &bytesSent, nullptr) || bytesSent < writeString_.length()) {
    LogError("Error writing to port");
    return false;
  }

  return true;
}

bool SerialCommunicationManager::PurgeBuffer() const {
  return PurgeComm(hSerial_, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, serialConfiguration_.port.c_str(), GetLastErrorAsString().c_str());
}

void SerialCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, serialConfiguration_.port.c_str());
}
