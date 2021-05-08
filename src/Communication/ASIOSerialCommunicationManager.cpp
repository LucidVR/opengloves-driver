#include "Communication/ASIOSerialCommunicationManager.h"
#include "DriverLog.h"

ASIOSerialCommunicationManager::ASIOSerialCommunicationManager(
    const VRSerialConfiguration_t &configuration,
    std::unique_ptr<IEncodingManager> encodingManager)
    : m_serialConfiguration(configuration),
      m_encodingManager(std::move(encodingManager)), m_io(), m_serialPort(m_io){};

bool ASIOSerialCommunicationManager::Connect() {
  asio::error_code ec;
  m_serialPort.open(m_serialConfiguration.port, ec);
  if (!ec) {
    m_serialPort.set_option(asio::serial_port::baud_rate(115200));

    return true;
  }

  DebugDriverLog("Received error connecting to port");

  return false;
}

void ASIOSerialCommunicationManager::BeginListener(const std::function<void(VRCommData_t)> &callback) {
  if (m_serialPort.is_open()) {
    m_active = true;
    m_serialThread = std::thread(&ASIOSerialCommunicationManager::ListenerThread, this, callback);
  }
}

void ASIOSerialCommunicationManager::ListenerThread(const std::function<void(VRCommData_t)> &callback) {
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  while (m_active) {

    std::string result;
    bool success = ReceiveNextPacket(result);

    if (success) {
      VRCommData_t data = m_encodingManager->Decode(result);

      callback(data);
    } else {
      DriverLog("Device disconnected unexpectedly!");
     // vr::VRServerDriverHost()->RequestRestart("One device has disconnected unexpectedly!", "", "", "");
      Disconnect();
    }
  }
}

bool ASIOSerialCommunicationManager::ReceiveNextPacket(std::string &result) {
  asio::streambuf buf;
  asio::error_code ec;

  asio::read_until(m_serialPort, buf, '\n', ec);

  if (!ec) {
    std::stringstream ss(asio::buffer_cast<const char *>(buf.data()));

    std::getline(ss, result);

    return true;
  }

  return false;
}

bool ASIOSerialCommunicationManager::IsConnected() {
  return m_serialPort.is_open();
}

void ASIOSerialCommunicationManager::Disconnect() {
  if (m_active) {
    m_active = false;

    m_serialThread.join();

    if (m_serialPort.is_open()) {
      m_serialPort.close();
    }
  }
}