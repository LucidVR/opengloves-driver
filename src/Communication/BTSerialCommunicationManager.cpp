#include <Communication/BTSerialCommunicationManager.h>
#include <string.h>

static const uint32_t c_listenerWaitTime = 1000;

// Adapted from Finally Functional's SerialBT implementation

static std::string GetLastErrorAsString() {
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string();
	}

	LPSTR messageBuffer = nullptr;

	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

BTSerialCommunicationManager::BTSerialCommunicationManager(const VRBTSerialConfiguration_t& configuration, std::unique_ptr<IEncodingManager> encodingManager)
	: m_btSerialConfiguration(configuration),
	m_encodingManager(std::move(encodingManager)),
	m_isConnected(false),
	m_btClientSocket(NULL),
	m_btSocketAddress(),
	m_deviceBtAddress(NULL),
	m_wcDeviceName(NULL) {};

bool BTSerialCommunicationManager::Connect() {
	DriverLog("Trying to connect to bluetooth");

	// We're not yet connected
	m_isConnected = false;

	// Try to connect
	if (!GetPairedDeviceBtAddress())  // find an device paired with this machine
	{
		DriverLog("Error getting Bluetooth address");
		return false;
	}
	if (!StartupWindowsSocket())  // initialize windows sockets
	{
		DriverLog("Error Initializing windows sockets");
		return false;
	}
	if (!ConnectToDevice())  // initialize BT windows socket for connecting to device
	{
		DriverLog("Error connecting to Bluetooth device");
		return false;
	}
	else {
		// If everything went fine we're connected
		m_isConnected = true;
		DriverLog("Connected to bluetooth!");
	}
	return true;
}

void BTSerialCommunicationManager::BeginListener(const std::function<void(VRCommData_t)>& callback) {
	m_threadActive = true;
	m_serialThread = std::thread(&BTSerialCommunicationManager::ListenerThread, this, callback);
}

void BTSerialCommunicationManager::ListenerThread(const std::function<void(VRCommData_t)>& callback) {
	WaitAttemptConnection();

	while (m_threadActive) {
		std::string receivedString;
		bool readSuccessful = ReceiveNextPacket(receivedString);
		if (readSuccessful) {
			try {
				VRCommData_t commData = m_encodingManager->Decode(receivedString);
				callback(commData);
				SendMessageToDevice();
			}
			catch (const std::invalid_argument& ia) {
				DriverLog("Received error from encoding manager: %s", ia.what());
			}
		}
		else {
			LogMessage("Detected device error. Disconnecting socket and attempting reconnection....");

			if (DisconnectFromDevice()) {
				WaitAttemptConnection();
				LogMessage("Successfully reconnected to device.");
				continue;
			}

			LogMessage("Could not disconnect. Closing listener...");
			Disconnect();
		}
	}
}

bool BTSerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
	char nextChar = 0;
	do {
		int recieveResult = recv(m_btClientSocket, &nextChar, 1, 0);
		if (recieveResult <= 0) continue;

		buff += nextChar;
	} while (nextChar != '\n' || buff.length() < 1);

	return true;
}

void BTSerialCommunicationManager::QueueSend(const VRFFBData_t& data) {
	std::lock_guard<std::mutex> lock(m_writeMutex);

	m_writeString = m_encodingManager->Encode(data);
}

void BTSerialCommunicationManager::Disconnect() {
	if (m_isConnected) {
		if (m_threadActive) {
			m_threadActive = false;
			m_serialThread.join();
		}
		DisconnectFromDevice();
	}
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

bool BTSerialCommunicationManager::IsConnected() { return m_isConnected; }

void BTSerialCommunicationManager::WaitAttemptConnection() {
	while (m_threadActive && !IsConnected() && !Connect()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(c_listenerWaitTime));
	}
}

bool BTSerialCommunicationManager::GetPairedDeviceBtAddress() {
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
	BLUETOOTH_DEVICE_INFO btDeviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO), 0 };  // default
	HBLUETOOTH_DEVICE_FIND btDevice = NULL;
	btDevice = BluetoothFindFirstDevice(&btDeviceSearchParameters, &btDeviceInfo);  // returns first BT device connected to this machine
	if (btDevice == NULL) {
		LogMessage("Could not find any bluetooth devices");
		return false;
	}
	do {
		std::wstring thiswstring = std::wstring(m_btSerialConfiguration.name.begin(), m_btSerialConfiguration.name.end());

		m_wcDeviceName = (WCHAR*)(thiswstring.c_str());
		if (wcscmp(btDeviceInfo.szName, m_wcDeviceName) == 0) {
			LogMessage("Bluetooth Device found");
			if (btDeviceInfo.fAuthenticated)  // I found that if fAuthenticated is true it means the device is paired.
			{
				LogMessage("Bluetooth Device is authenticated");
				m_deviceBtAddress = btDeviceInfo.Address.ullLong;
				return true;
			}
			else {
				LogMessage("This Bluetooth Device is not authenticated. Please pair with it first");
			}
		}
	} while (BluetoothFindNextDevice(btDevice, &btDeviceInfo));  // loop through remaining BT devices connected to this machine

	LogMessage("Could not find paired Bluetooth Device");
	return false;
}

bool BTSerialCommunicationManager::StartupWindowsSocket() {
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	int wsaStartupError = WSAStartup(wVersionRequested, &wsaData);  // call this before using BT windows socket.
	if (wsaStartupError != 0) {
		LogMessage("WSA failed to startup");
		return false;
	}
	return true;
}

bool BTSerialCommunicationManager::ConnectToDevice() {
	m_btClientSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);  // initialize BT windows socket
	memset(&m_btSocketAddress, 0, sizeof(m_btSocketAddress));
	m_btSocketAddress.addressFamily = AF_BTH;
	m_btSocketAddress.serviceClassId = RFCOMM_PROTOCOL_UUID;
	m_btSocketAddress.port = 0;                                                                    // port needs to be 0 if the remote device is a client. See references.
	m_btSocketAddress.btAddr = m_deviceBtAddress;                                                  // this is the BT address of the remote device.
	if (connect(m_btClientSocket, (SOCKADDR*)&m_btSocketAddress, sizeof(m_btSocketAddress)) != 0)  // connect to the BT device.
	{
		LogError("Could not connect socket to Bluetooth Device");
		return false;
	}

	unsigned long nonBlockingMode = 1;
	if (ioctlsocket(m_btClientSocket, FIONBIO, (unsigned long*)&nonBlockingMode) != 0)  // set the socket to be non-blocking, meaning
	{                                                                                   // it will return right away when sending/recieving
		LogError("Could not set socket to be non-blocking");
		return false;
	}

	return true;
}

bool BTSerialCommunicationManager::SendMessageToDevice() {
	std::lock_guard<std::mutex> lock(m_writeMutex);
	const char* message = m_writeString.c_str();
	int sendResult = send(m_btClientSocket, message, (int)strlen(message), 0);  // send your message to the BT device
	if (sendResult == SOCKET_ERROR) {
		LogError("Sending to Bluetooth Device failed");

		closesocket(m_btClientSocket);
		WSACleanup();

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