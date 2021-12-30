#include "Communication/BTSerialCommunicationManager.h"

#include <Windows.h>
#include <ws2bth.h>

#include <utility>

#include "DriverLog.h"
#include "Util/Logic.h"
#include "Util/Windows.h"

BTSerialCommunicationManager::BTSerialCommunicationManager(
    std::unique_ptr<EncodingManager> encodingManager, const VRBTSerialConfiguration configuration, const VRDeviceConfiguration& deviceConfiguration)
    : CommunicationManager(std::move(encodingManager), deviceConfiguration),
      btSerialConfiguration_(configuration),
      isConnected_(false),
      btClientSocket_(NULL) {}

bool BTSerialCommunicationManager::IsConnected() {
  return isConnected_;
}

bool BTSerialCommunicationManager::Connect() {
  // We're not yet connected
  isConnected_ = false;

  BTH_ADDR deviceBtAddress;
  if (!GetPairedDeviceBtAddress(&deviceBtAddress) || !StartupWindowsSocket() || !ConnectToDevice(deviceBtAddress)) {
    LogMessage("Failed to connect to device");
    return false;
  }

  // If everything went fine we're connected
  isConnected_ = true;
  LogMessage("Connected to bluetooth");
  return true;
}

bool BTSerialCommunicationManager::DisconnectFromDevice() {
  if (shutdown(btClientSocket_, 2) == SOCKET_ERROR) {
    LogMessage("Could not disconnect socket from bluetooth device");
    return false;
  }

  isConnected_ = false;
  LogMessage("Disconnected from socket successfully");
  return true;
}

bool BTSerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
  char nextChar = 0;
  do {
    const int receiveResult = recv(btClientSocket_, &nextChar, 1, 0);
    if (receiveResult <= 0 || nextChar == '\n') continue;

    buff += nextChar;
  } while (threadActive_ && (nextChar != '\n' || buff.length() < 1));

  if (!threadActive_) return false;

  return true;
}

bool BTSerialCommunicationManager::SendMessageToDevice() {
  std::lock_guard lock(writeMutex_);

  const char* message = writeString_.c_str();

  if (!Retry([&]() { return send(btClientSocket_, message, static_cast<int>(writeString_.length()), 0) != SOCKET_ERROR; }, 5, 10)) {
    LogError("Sending to Bluetooth Device failed... closing");

    closesocket(btClientSocket_);
    WSACleanup();
  }

  return true;
}

bool BTSerialCommunicationManager::ConnectToDevice(const BTH_ADDR& deviceBtAddress) {
  btClientSocket_ = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);  // initialize BT windows socket

  SOCKADDR_BTH btSocketAddress{};
  btSocketAddress.addressFamily = AF_BTH;
  btSocketAddress.serviceClassId = RFCOMM_PROTOCOL_UUID;
  btSocketAddress.port = 0;                  // port needs to be 0 if the remote device is a client. See references.
  btSocketAddress.btAddr = deviceBtAddress;  // this is the BT address of the remote device.

  if (connect(btClientSocket_, reinterpret_cast<SOCKADDR*>(&btSocketAddress), sizeof btSocketAddress) != 0)  // connect to the BT device.
  {
    LogError("Could not connect socket to Bluetooth Device");
    return false;
  }

  unsigned long nonBlockingMode = 1;
  // set the socket to be non-blocking, meaning it will return right away when sending/receiving
  if (ioctlsocket(btClientSocket_, FIONBIO, &nonBlockingMode) != 0) {
    LogError("Could not set socket to be non-blocking");
    return false;
  }

  return true;
}

bool BTSerialCommunicationManager::GetPairedDeviceBtAddress(BTH_ADDR* deviceBtAddress) {
  constexpr BLUETOOTH_DEVICE_SEARCH_PARAMS btDeviceSearchParameters = {
      sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),  // size of object
      1,                                       // return authenticated devices
      0,                                       // return remembered devices
      1,                                       // return unknown devices
      1,                                       // return connected devices
      1,                                       // issue in query
      2,                                       // timeout multiplier. Multiply this value by 1.28 seconds to get timeout.
      nullptr                                  // radio handler
  };

  BLUETOOTH_DEVICE_INFO btDeviceInfo = {sizeof(BLUETOOTH_DEVICE_INFO), {0}};  // default
  const HBLUETOOTH_DEVICE_FIND btDevice =
      BluetoothFindFirstDevice(&btDeviceSearchParameters, &btDeviceInfo);  // returns first BT device connected to this machine

  if (btDevice == nullptr) {
    LogMessage("Could not find any bluetooth devices");
    *deviceBtAddress = NULL;
    return false;
  }

  const auto thiswstring = std::wstring(btSerialConfiguration_.name.begin(), btSerialConfiguration_.name.end());
  const WCHAR* wcDeviceName = const_cast<WCHAR*>(thiswstring.c_str());

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
  constexpr WORD wVersionRequested = MAKEWORD(2, 2);
  WSADATA wsaData;

  if (const int wsaStartupError = WSAStartup(wVersionRequested, &wsaData); wsaStartupError != 0) {
    LogMessage("WSA failed to startup");
    return false;
  }

  return true;
}

void BTSerialCommunicationManager::LogError(const char* message) {
  // message with port name and last error
  DriverLog("%s (%s) - Error: %s", message, btSerialConfiguration_.name.c_str(), GetLastErrorAsString().c_str());
}

void BTSerialCommunicationManager::LogMessage(const char* message) {
  // message with port name
  DriverLog("%s (%s)", message, btSerialConfiguration_.name.c_str());
}
