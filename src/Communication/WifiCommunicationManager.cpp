#include <Communication/WifiCommunicationManager.h>

// Adapted from Finally Functional's SerialBT implementation

WifiCommunicationManager::WifiCommunicationManager(
    const VRBTSerialConfiguration_t& configuration,
    std::unique_ptr<IEncodingManager> encodingManager)
    : m_btSerialConfiguration(configuration),
      m_encodingManager(std::move(encodingManager)),
      m_isConnected(false){
          // convert the bluetooth device name from settings into wide
          // const char* name = configuration.name.c_str();
          // size_t newsize = strlen(name) + 1;
          // m_wcDeviceName = new WCHAR[newsize];
          // size_t convertedChars = 0;
          // mbstowcs_s(&convertedChars, m_wcDeviceName, newsize, name, _TRUNCATE);

          // std::wstring thiswstring = std::wstring(configuration.name.begin(),
          // configuration.name.end());

          // m_wcDeviceName = (WCHAR*)(thiswstring.c_str());

      };

void WifiCommunicationManager::Connect() {}

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
bool WifiCommunicationManager::connectToEsp32() { WifiCommunicationManager}

bool WifiCommunicationManager::sendMessageToEsp32() {
  
}