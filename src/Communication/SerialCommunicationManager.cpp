#include <Communication/SerialCommunicationManager.h>

#include <chrono>

#include "DriverLog.h"

void SerialCommunicationManager::Connect() {
  // We're not yet connected
  m_isConnected = false;

  // Try to connect to the given port throuh CreateFile
  m_hSerial = CreateFile(m_serialConfiguration.port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (this->m_hSerial == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
      DriverLog("Handle was not attached. Reason: not available.");
    } else {
      DriverLog("Received error connecting to port");
    }
  } else {
    // If connected we try to set the comm parameters
    DCB dcbSerialParams = {0};

    // Try to get the current
    if (!GetCommState(m_hSerial, &dcbSerialParams)) {
      // If impossible, show an error
      DriverLog("Failed to get current serial parameters!");
    } else {
      // Define serial connection parameters for the arduino board
      dcbSerialParams.BaudRate = CBR_115200;
      dcbSerialParams.ByteSize = 8;
      dcbSerialParams.StopBits = ONESTOPBIT;
      dcbSerialParams.Parity = NOPARITY;

      // reset upon establishing a connection
      dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

      // set the parameters and check for their proper application
      if (!SetCommState(m_hSerial, &dcbSerialParams)) {
        DriverLog("Could not set Serial Port parameters");
      } else {
        // If everything went fine we're connected
        m_isConnected = true;

        // Flush any remaining characters in the buffers
        PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
      }
    }
  }
}

void SerialCommunicationManager::BeginListener(const std::function<void(VRCommData_t)>& callback) {
  // DebugDriverLog("Begun listener");
  m_threadActive = true;
  m_serialThread = std::thread(&SerialCommunicationManager::ListenerThread, this, callback);
}

void SerialCommunicationManager::ListenerThread(const std::function<void(VRCommData_t)>& callback) {
  // DebugDriverLog("In listener thread");
  std::this_thread::sleep_for(std::chrono::milliseconds(ARDUINO_WAIT_TIME));
  PurgeBuffer();

  while (m_threadActive) {
    std::string receivedString;
    bool readSuccessful = ReceiveNextPacket(receivedString);

    if (readSuccessful) {
      try {
        VRCommData_t commData = m_encodingManager->Decode(receivedString);
        callback(commData);

        // Then write the data we have queued
        Write();
      } catch (const std::invalid_argument& ia) {
        DriverLog("Received error decoding data. Skipping...");
      }
    } else {
      DebugDriverLog("Detected that arduino has disconnected! Stopping listener...");
      // We should probably do more logic for trying to reconnect to the arduino
      // For now, it should be obvious to people that the arduinos have disconnected
      m_threadActive = false;
    }
  }
}

void SerialCommunicationManager::QueueSend(const std::string& data) {
  std::lock_guard<std::mutex> lock(m_queuedSendDataMutex);

  m_queuedSendData = data;
}

bool SerialCommunicationManager::Write() {
  DWORD bytesSend;

  if (!WriteFile(m_hSerial, (void*)m_queuedSendData.c_str(), m_queuedSendData.size(), &bytesSend,
                 0)) {
    ClearCommError(m_hSerial, &m_errors, &m_status);

    DebugDriverLog("Success writing to serial.");
  } else {
    DriverLog("Error writing to serial port.");
  }
}

bool SerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  DWORD dwCommEvent;
  DWORD dwRead = 0;

  if (!SetCommMask(m_hSerial, EV_RXCHAR)) {
    DebugDriverLog("Error setting comm mask");
    return false;
  }

  char nextChar;
  int bytesRead = 0;
  if (WaitCommEvent(m_hSerial, &dwCommEvent, NULL)) {
    do {
      if (ReadFile(m_hSerial, &nextChar, 1, &dwRead, NULL)) {
        buff += nextChar;
        bytesRead++;
      } else {
        DebugDriverLog("Read file error");
        return false;
      }
    } while (nextChar != '\n');
  } else {
    DebugDriverLog("Error in comm event");
    return false;
  }

  return true;
}
bool SerialCommunicationManager::PurgeBuffer() {
  return PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialCommunicationManager::Disconnect() {
  if (m_isConnected) {
    if (m_threadActive) {
      m_threadActive = false;
      m_serialThread.join();
    }
    m_isConnected = false;

    // Disconnect
    CloseHandle(m_hSerial);
  }
}
// May want to get a heartbeat here instead?
bool SerialCommunicationManager::IsConnected() { return m_isConnected; }