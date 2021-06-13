#include <Communication/WifiCommunicationManager.h>

// Adapted from Finally Functional's SerialBT implementation

WifiCommunicationManager::WifiCommunicationManager(const VRBTSerialConfiguration_t& configuration, std::unique_ptr<IEncodingManager> encodingManager) : m_isConnected(false) {
    m_endpoint.set_error_channels(websocketpp::log::elevel::all);
    m_endpoint.set_message_handler(&on_message);
    m_endpoint.set_access_channels(websocketpp::log::alevel::all ^  websocketpp::log::alevel::frame_payload);
    m_endpoint.init_asio();
};

void WifiCommunicationManager::Connect() {
  m_endpoint.listen(9002);
  m_endpoint.start_accept();
  m_endpoint.run();
}

void on_message(websocketpp::connection_hdl, server::message_ptr msg) {
  DriverLog(msg->get_payload().c_str());
}

void WifiCommunicationManager::BeginListener(
    const std::function<void(VRCommData_t)>& callback) {

}

void WifiCommunicationManager::ListenerThread(
    const std::function<void(VRCommData_t)>& callback) {

}

bool WifiCommunicationManager::ReceiveNextPacket(std::string& buff) {

}

void WifiCommunicationManager::Disconnect() {

}
// May want to get a heartbeat here instead?
bool WifiCommunicationManager::IsConnected() { return m_isConnected; }

/// <summary>
/// Gets the bluetooth devices paired with this machine and
/// finds an ESP32. If it finds one, its BT address is stored.
/// </summary>
bool WifiCommunicationManager::getPairedEsp32BtAddress() {
  
}

/// <summary>
/// Windows sockets need an initialization method called before they are used.
/// </summary>
bool WifiCommunicationManager::startupWindowsSocket() {
  
}

/// <summary>
/// Sets up bluetooth socket to communicate with ESP32.
/// </summary>
bool WifiCommunicationManager::connectToEsp32() {
}

bool WifiCommunicationManager::sendMessageToEsp32() {
  
}