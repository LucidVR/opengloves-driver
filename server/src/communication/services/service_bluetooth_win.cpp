// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

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

BluetoothCommunicationService::BluetoothCommunicationService(og::DeviceBluetoothCommunicationConfiguration configuration)
    : configuration_(std::move(configuration)) {
  Connect();
}

void BluetoothCommunicationService::LogError(const std::string& message, bool with_win_error = true) const {
  std::ostringstream oss;

  logger.Log(kLoggerLevel_Error, "%s, %s: %s", configuration_.name.c_str(), message.c_str(), with_win_error ? GetLastErrorAsString().c_str() : "");
}

bool BluetoothCommunicationService::IsConnected() {
  return is_connected_;
}

bool BluetoothCommunicationService::Connect() {
  std::scoped_lock lock(io_mutex_);
  WSAData data{};

  if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
    LogError("WSA failed to startup");

    return false;
  }

  // find by name
  BLUETOOTH_DEVICE_SEARCH_PARAMS btDeviceSearchParameters = {
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

  BTH_ADDR device_address = 0;
  if (btDevice == nullptr) {
    logger.Log(og::kLoggerLevel_Warning, "Could not find any bluetooth devices");
    device_address = NULL;
    return false;
  }

  const auto thiswstring = std::wstring(configuration_.name.begin(), configuration_.name.end());
  const WCHAR* wcDeviceName = const_cast<WCHAR*>(thiswstring.c_str());

  bool found_device = false;
  do {
    if (wcscmp(btDeviceInfo.szName, wcDeviceName) == 0) {
      if (btDeviceInfo.fAuthenticated)  // device is paired
      {
        device_address = btDeviceInfo.Address.ullLong;
        found_device = true;
      } else {
        logger.Log(og::kLoggerLevel_Warning, "This Bluetooth device is not authenticated. Please pair with it first");
        found_device = false;
      }
    }
  } while (BluetoothFindNextDevice(btDevice, &btDeviceInfo));  // loop through remaining BT devices connected to this machine

  if (!found_device) return false;

  sock_ = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

  SOCKADDR_BTH sock_address{};
  sock_address.addressFamily = AF_BTH;
  sock_address.serviceClassId = RFCOMM_PROTOCOL_UUID;
  sock_address.port = 0;
  sock_address.btAddr = device_address;

  if (connect(sock_, reinterpret_cast<SOCKADDR*>(&sock_address), sizeof sock_address) != 0) {
    LogError("Failed to connect to bluetooth device");

    return false;
  }

  DWORD timeout = 1000;
  setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof timeout);
  setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof timeout);

  is_connected_ = true;

  return true;
}

bool BluetoothCommunicationService::ReceiveNextPacket(std::string& buff) {
  if (!is_connected_) return false;

  char next_char = 0;

  do {
    std::scoped_lock lock(io_mutex_);
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
  if (!is_connected_) return false;

  const char* cbuff = buff.c_str();

  std::scoped_lock lock(io_mutex_);
  if (send(sock_, cbuff, strlen(cbuff), 0) < 0) {
    LogError("Failed to send data to bluetooth device");

    return false;
  }

  return true;
}

bool BluetoothCommunicationService::PrepareDisconnect() {
  return true;
}

BluetoothCommunicationService::~BluetoothCommunicationService() {
  is_disconnecting_ = true;
  std::scoped_lock lock(io_mutex_);
  if (is_connected_.exchange(false)) {
    if (shutdown(sock_, SD_BOTH) == SOCKET_ERROR) {
      LogError("Failed to disconnect from bluetooth socket device");
    }

    is_connected_ = false;
  }
}

std::string BluetoothCommunicationService::GetIdentifier() {
  return configuration_.name;
}