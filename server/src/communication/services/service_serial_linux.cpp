#include "service_serial_linux.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

#include <cstring>
#include <utility>

static og::Logger& logger = og::Logger::GetInstance();

SerialCommunicationService::SerialCommunicationService(og::DeviceSerialCommunicationConfiguration configuration)
    : configuration_(std::move(configuration)), is_connected_(false) {
  int fd = open(configuration_.port_name.c_str(), O_RDWR);

  // error opening file
  if (fd < 0) {
    logger.Log(og::kLoggerLevel_Error, "Failed to open port! %s", strerror(errno));

    return;
  }

  // read existing settings
  struct termios tty{};
  if (tcgetattr(fd, &tty) != 0) {
    logger.Log(og::kLoggerLevel_Error, "Failed to read port settings! %s", strerror(errno));

    return;
  }

  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag |= CREAD | CLOCAL;
  tty.c_cflag &= ~CRTSCTS;

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ECHOE;
  tty.c_lflag &= ~ECHONL;
  tty.c_lflag &= ~ISIG;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;

  tty.c_cc[VTIME] = 10;
  tty.c_cc[VMIN] = 0;

  // baud rates
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    logger.Log(og::kLoggerLevel_Error, "Failed to set port settings! %s", strerror(errno));

    return;
  }

  is_connected_ = true;
}

bool SerialCommunicationService::ReceiveNextPacket(std::string& buff) {
  char next_char;

  do {
    // try read one byte
    int ret = read(fd_, &next_char, 1);
    if (ret < 0) {
      logger.Log(og::kLoggerLevel_Error, "Failed to read from device! %s", strerror(ret));
      return false;
    }

    if (next_char == 0 || next_char == '\n') continue;

    buff += next_char;
  } while (next_char != '\n');

  return true;
}

bool SerialCommunicationService::RawWrite(const std::string& buff) {
  if (int ret = write(fd_, buff.c_str(), buff.size()) < 0) {
    logger.Log(og::kLoggerLevel_Error, "Failed to write to device! %s", strerror(ret));

    return false;
  }

  return true;
}

bool SerialCommunicationService::IsConnected() {
  return is_connected_;
}

std::string SerialCommunicationService::GetIdentifier() {
  return "serial";
}
SerialCommunicationService::~SerialCommunicationService() = default;
