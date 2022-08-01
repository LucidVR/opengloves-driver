#include "service_bluetooth_win.h"

#include <Windows.h>
#include <ws2bth.h>

#include <sstream>

#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

static std::string GetLastErrorAsString() {
  const DWORD errorMessageId = ::WSAGetLastError();
  if (errorMessageId == 0) return std::string();

  LPSTR messageBuffer = nullptr;
  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      errorMessageId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&messageBuffer),
      0,
      nullptr);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}

BluetoothCommunicationService::BluetoothCommunicationService(BTH_ADDR bt_address) {
  bt_address_ = bt_address;

  Connect();
}

void BluetoothCommunicationService::LogError(const std::string& message, bool with_win_error = true) {
  std::ostringstream oss;

  logger.Log(kLoggerLevel_Error, "%llu, %s: %s", bt_address_, message.c_str(), with_win_error ? GetLastErrorAsString().c_str() : "");
}

bool BluetoothCommunicationService::IsConnected() {
  return true;
}

bool BluetoothCommunicationService::Connect() {
  WSAData data;

  if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
    LogError("WSA failed to startup");

    return false;
  }

  sock_ = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

  SOCKADDR_BTH sock_address{};
  sock_address.addressFamily = AF_BTH;
  sock_address.serviceClassId = RFCOMM_PROTOCOL_UUID;
  sock_address.port = 0;
  sock_address.btAddr = bt_address_;

  if (connect(sock_, reinterpret_cast<SOCKADDR*>(&sock_address), sizeof sock_address) != 0) {
    LogError("Failed to connect to bluetooth device");

    return false;
  }

  DWORD timeout = 100;
  setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof timeout);
  setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof timeout);

  is_connected_ = true;

  return true;
}

bool BluetoothCommunicationService::ReceiveNextPacket(std::string& buff) {
  char next_char = 0;

  do {
    const int err = recv(sock_, &next_char, 1, 0);

    if (err == SOCKET_ERROR) {
      LogError("Received socket error reading next byte from bluetooth");

      return false;
    }

    buff += next_char;
  } while (next_char != '\n' && !is_disconnecting_);

  return true;
}

bool BluetoothCommunicationService::RawWrite(const std::string& buff) {
  const char* cbuff = buff.c_str();

  if (send(sock_, cbuff, strlen(cbuff), 0) < 0) {
    LogError("Failed to send data to bluetooth device");

    return false;
  }

  return true;
}

bool BluetoothCommunicationService::PurgeBuffer() {
  logger.Log(og::kLoggerLevel_Info, "Attempting to purge bluetooth buffer");

  char buff[100];
  int bytes_read;
  do {
    bytes_read = recv(sock_, buff, strlen(buff), 0);

  } while (bytes_read > 0 && !is_disconnecting_);

  logger.Log(og::kLoggerLevel_Info, "Purged bluetooth buffer");

  return true;
}

bool BluetoothCommunicationService::Disconnect() {
  if (shutdown(sock_, SD_BOTH) != 0) {
    LogError("Failed to disconnect from bluetooth socket device");

    return false;
  }

  is_connected_ = false;

  return true;
}

BluetoothCommunicationService::~BluetoothCommunicationService() {
  logger.Log(og::kLoggerLevel_Info, "Attempting to disconnect from bluetooth device");

  is_disconnecting_ = true;
  Disconnect();

  logger.Log(og::kLoggerLevel_Info, "Successfully disconnected from bluetooth device");
}