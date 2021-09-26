#include "Communication/BTSerialCommunicationManager.h"

#include <Windows.h>
#include <Ws2bth.h>

#include "DriverLog.h"
#include "Util/Logic.h"
#include "Util/Windows.h"

BTSerialCommunicationManager::BTSerialCommunicationManager(std::unique_ptr<EncodingManager> encodingManager, const VRBTSerialConfiguration& configuration)
    : CommunicationManager(std::move(encodingManager)), m_btSerialConfiguration(configuration), m_isConnected(false), m_btClientSocket(NULL) {}

bool BTSerialCommunicationManager::IsConnected() { return m_isConnected; }

bool BTSerialCommunicationManager::Connect() {
  // We're not yet connected
  m_isConnected = false;

  BTH_ADDR deviceBtAddress;
  if (!GetPairedDeviceBtAddress(&deviceBtAddress) || !StartupWindowsSocket() || !ConnectToDevice(deviceBtAddress)) {
    LogMessage("Failed to connect to device");
    return false;
  }

  // If everything went fine we're connected
  m_isConnected = true;
  LogMessage("Connected to bluetooth");
  return true;
}

bool BTSerialCommunicationManager::DisconnectFromDevice() {
  if (shutdown(m_btClientSocket, 2) == SOCKET_ERROR) {
    LogMessage("Could not disconnect socket from bluetooth device");
    return false;
  }

  m_isConnected = false;
  LogMessage("Disconnected from socket successfully");
  return true;
}

bool BTSerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  char nextChar = 0;
  do {
    int recieveResult = recv(m_btClientSocket, &nextChar, 1, 0);
    if (recieveResult <= 0 || nextChar == '\n') continue;

    buff += nextChar;
  } while (m_threadActive && (nextChar != '\n' || buff.length() < 1));

  if (!m_threadActive) return false;

  return true;
}

bool BTSerialCommunicationManager::SendMessageToDevice() {
  std::lock_guard<std::mutex> lock(m_writeMutex);

  const char* message = m_writeString.c_str();

  if (!retry([&]() { return send(m_btClientSocket, message, (int)m_writeString.length(), 0) != SOCKET_ERROR; }, 5, 10)) {
    LogError("Sending to Bluetooth Device failed... closing");

    closesocket(m_btClientSocket);
    WSACleanup();
  }

  return true;
}

bool BTSerialCommunicationManager::ConnectToDevice(BTH_ADDR& deviceBtAddress) {
  m_btClientSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);  // initialize BT windows socket

  SOCKADDR_BTH m_btSocketAddress{};
  m_btSocketAddress.addressFamily = AF_BTH;
  m_btSocketAddress.serviceClassId = RFCOMM_PROTOCOL_UUID;
  m_btSocketAddress.port = 0;                  // port needs to be 0 if the remote device is a client. See references.
  m_btSocketAddress.btAddr = deviceBtAddress;  // this is the BT address of the remote device.

  if (connect(m_btClientSocket, (SOCKADDR*)&m_btSocketAddress, sizeof(m_btSocketAddress)) != 0)  // connect to the BT device.
  {
    LogError("Could not connect socket to Bluetooth Device");
    return false;
  }

  unsigned long nonBlockingMode = 1;
  if (ioctlsocket(m_btClientSocket, FIONBIO, &nonBlockingMode) != 0)  // set the socket to be non-blocking, meaning
  {                                                                   // it will return right away when sending/recieving
    LogError("Could not set socket to be non-blocking");
    return false;
  }

  return true;
}

bool BTSerialCommunicationManager::GetPairedDeviceBtAddress(BTH_ADDR* deviceBtAddress) {
  BLUETOOTH_DEVICE_SEARCH_PARAMS btDeviceSearchParameters = {
      sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),  // size of object
      1,                                       // return authenticated devices
      0,                                       // return remembered devices
      1,                                       // return unknown devices
      1,                                       // return connected devices
      1,                                       // issue inquery
      2,                                       // timeout multipler. Multiply this value by 1.28 seconds to get timeout.
      NULL                                     // radio handler
  };
  BLUETOOTH_DEVICE_INFO btDeviceInfo = {sizeof(BLUETOOTH_DEVICE_INFO), 0};  // default
  HBLUETOOTH_DEVICE_FIND btDevice = NULL;
  btDevice = BluetoothFindFirstDevice(&btDeviceSearchParameters, &btDeviceInfo);  // returns first BT device connected to this machine
  if (btDevice == NULL) {
    LogMessage("Could not find any bluetooth devices");
    *deviceBtAddress = NULL;
    return false;
  }

  std::wstring thiswstring = std::wstring(m_btSerialConfiguration.name.begin(), m_btSerialConfiguration.name.end());
  WCHAR* wcDeviceName = (WCHAR*)(thiswstring.c_str());

  do {
    if (wcscmp(btDeviceInfo.szName, wcDeviceName) == 0) {
      LogMessage("Bluetooth Device found");
      if (btDeviceInfo.fAuthenticated)  // I found that if fAuthenticated is true it means the device is paired.
      {
        LogMessage("Bluetooth Device is authenticated");
        *deviceBtAddress = btDeviceInfo.Address.ullLong;
        return true;
      } else {
        LogMessage("This Bluetooth Device is not authenticated. Please pair with it first");
      }
    }
  } while (BluetoothFindNextDevice(btDevice, &btDeviceInfo));  // loop through remaining BT devices connected to this machine

  LogMessage("Could not find paired Bluetooth Device");
  *deviceBtAddress = NULL;
  return false;
}

bool BTSerialCommunicationManager::StartupWindowsSocket() {
  WORD wVersionRequested = MAKEWORD(2, 2);
  WSADATA wsaData;
  int wsaStartupError = WSAStartup(wVersionRequested, &wsaData);  // call this before using BT windows socket.
  if (wsaStartupError != 0) {
    LogMessage("WSA failed to startup");
    return false;
  }
  return true;
}

void BTSerialCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, m_btSerialConfiguration.name.c_str(), GetLastErrorAsString().c_str());
}

void BTSerialCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, m_btSerialConfiguration.name.c_str());
}