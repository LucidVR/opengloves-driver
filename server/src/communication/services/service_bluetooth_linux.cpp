#include "service_bluetooth_linux.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <utility>

static og::Logger& logger = og::Logger::GetInstance();

BluetoothCommunicationService::BluetoothCommunicationService(og::DeviceBluetoothCommunicationConfiguration configuration)
    : configuration_(std::move(configuration)), is_connected_(false), socket_(0) {
  socket_ = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  struct sockaddr_rc addr = {0};
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t)1;
  str2ba(configuration_.name.c_str(), &addr.rc_bdaddr);

  // connect to server
  int ret = connect(socket_, (struct sockaddr*)&addr, sizeof(addr));

  if (ret < 0) {
    logger.Log(og::kLoggerLevel_Error, "Failed to connect to bluetooth device! %s", strerror(errno));

    is_connected_ = false;

    return;
  }

  is_connected_ = true;
};

bool BluetoothCommunicationService::ReceiveNextPacket(std::string& buff) {
  char next_char;
  do {
    // try read one byte
    int ret = read(socket_, &next_char, 1);
    if (ret < 0) {
      logger.Log(og::kLoggerLevel_Error, "Failed to read from device! %s", strerror(ret));
      return false;
    }

    if (next_char == 0 || next_char == '\n') continue;

    buff += next_char;
  } while (next_char != '\n');

  return true;
}

bool BluetoothCommunicationService::RawWrite(const std::string& buff) {
  if (int ret = write(socket_, buff.c_str(), buff.size()) < 0) {
    logger.Log(og::kLoggerLevel_Error, "Failed to write to device! %s", strerror(ret));

    return false;
  }

  return true;
}

bool BluetoothCommunicationService::IsConnected() {
  return is_connected_;
}

std::string BluetoothCommunicationService::GetIdentifier() {
  return "bluetooth";
}

void BluetoothCommunicationService::Disconnect() {
  if (!is_connected_.exchange(false) || socket_ == 0) {
    // we weren't connected
    return;
  }

  close(socket_);
  socket_ = 0;
}

BluetoothCommunicationService::~BluetoothCommunicationService() {
  Disconnect();
}