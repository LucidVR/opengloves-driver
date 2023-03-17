// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "service_serial_win.h"

#include "opengloves_interface.h"

using namespace og;

static Logger& logger = Logger::GetInstance();

#define ERROR_DISCONNECT_AND_RETURN(str_message) \
  {                                              \
    LogError(str_message);                       \
    Disconnect();                                \
    return false;                                \
  }

static std::string GetLastErrorAsString() {
  const DWORD error_id = ::GetLastError();
  if (error_id == 0) return "";

  LPSTR message_buffer = nullptr;
  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      error_id,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&message_buffer),
      0,
      nullptr);

  std::string message(message_buffer, size);

  LocalFree(message_buffer);

  return message;
}

void SerialCommunicationService::LogError(const std::string& message, bool with_win_error = true) {
  logger.Log(
      kLoggerLevel_Error, "%s, %s: %s", configuration_.port_name.c_str(), message.c_str(), with_win_error ? GetLastErrorAsString().c_str() : "");
}

SerialCommunicationService::SerialCommunicationService(og::DeviceSerialCommunicationConfiguration configuration)
    : configuration_(std::move(configuration)) {
  Connect();
}

bool SerialCommunicationService::IsConnected() {
  return is_connected_;
}

bool SerialCommunicationService::Connect() {
  is_connected_ = false;

  handle_ = CreateFile(configuration_.port_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (handle_ == INVALID_HANDLE_VALUE) return false;

  DCB serial_params{};

  if (!GetCommState(handle_, &serial_params)) ERROR_DISCONNECT_AND_RETURN("Failed to get current port parameters");

  serial_params.BaudRate = 115200;
  serial_params.ByteSize = 8;
  serial_params.StopBits = ONESTOPBIT;
  serial_params.fParity = NOPARITY;
  serial_params.fDtrControl = DTR_CONTROL_ENABLE;

  if (!SetCommState(handle_, &serial_params)) ERROR_DISCONNECT_AND_RETURN("Failed to set serial parameters");

  COMMTIMEOUTS timeout;
  timeout.ReadIntervalTimeout = 50;
  timeout.ReadTotalTimeoutConstant = 0;
  timeout.ReadTotalTimeoutMultiplier = 0;
  timeout.WriteTotalTimeoutConstant = 50;
  timeout.WriteTotalTimeoutMultiplier = 0;
  if (!SetCommTimeouts(handle_, &timeout)) ERROR_DISCONNECT_AND_RETURN("Failed to set port timeouts");

  if (!SetupComm(handle_, 200, 200)) ERROR_DISCONNECT_AND_RETURN("Failed to set port buffer size");

  logger.Log(og::kLoggerLevel_Info, "Successfully connected to COM port: %s", configuration_.port_name.c_str());

  is_connected_ = true;

  return true;
}

bool SerialCommunicationService::ReceiveNextPacket(std::string& buff) {
  if (!is_connected_) {
    LogError("Cannot receive packet as not connected to device", false);
    return false;
  }

  if (!SetCommMask(handle_, EV_RXCHAR)) {
    LogError("Error setting comm mask");
    return false;
  }

  DWORD dwCommEvent = 0;
  do {
    if (!WaitCommEvent(handle_, &dwCommEvent, nullptr)) {
      LogError("Error waiting for event");
      return false;
    }
  } while (is_connected_ && (dwCommEvent & EV_RXCHAR) != EV_RXCHAR);

  char next_char = 0;
  DWORD bytes_read = 0;
  do {
    if (!ReadFile(handle_, &next_char, 1, &bytes_read, nullptr)) {
      LogError("Failed to read from serial port");

      return false;
    }

    if (bytes_read <= 0 || next_char == '\n') continue;

    buff += next_char;

  } while (next_char != '\n' && !is_disconnecting_ && is_connected_);

  return true;
}

bool SerialCommunicationService::RawWrite(const std::string& buff) {
  if (!is_connected_) {
    LogError("Cannot write to device as it is not connected", false);

    return false;
  }

  const char* cbuff = buff.c_str();
  DWORD bytes_sent;

  if (!WriteFile(handle_, cbuff, strlen(cbuff), &bytes_sent, nullptr)) {
    LogError("Failed to write to serial port");

    return false;
  }

  return true;
}

bool SerialCommunicationService::PrepareDisconnect() {
  CancelIO();
  return true;
}

bool SerialCommunicationService::CancelIO() {
  if (!is_connected_) {
    LogError("Cannot cancel io as device is not connected", false);
    return false;
  }

  if (!CancelIoEx(handle_, nullptr)) {
    LogError("Failed to cancel serial port io");

    return false;
  }

  return true;
}

bool SerialCommunicationService::Disconnect() {
  if (!is_connected_) {
    LogError("Cannot disconnect from device is not connected", false);
    return false;
  }

  if (!CloseHandle(handle_)) {
    LogError("Failed to close serial port handle");

    return false;
  }

  is_connected_ = false;

  logger.Log(og::kLoggerLevel_Info, "Successfully disconnected from serial port");
  return true;
}

SerialCommunicationService::~SerialCommunicationService() {
  if (!is_connected_) return;

  is_disconnecting_ = true;

  CancelIO();
  Disconnect();

  logger.Log(og::kLoggerLevel_Info, "Closing serial port communication service");
}

std::string SerialCommunicationService::GetIdentifier() {
  return configuration_.port_name;
}