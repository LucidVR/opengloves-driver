#include <Communication/BTSerialCommunicationManager.h>

//Adapted from Finally Functional's SerialBT implementation

void BTSerialCommunicationManager::Connect() {
	DriverLog("Trying to connect to bluetooth");
	
	//We're not yet connected
	m_isConnected = false;

	//Try to connect
	if (!getPairedEsp32BtAddress()) //find an ESP32 paired with this machine
	{
		DebugDriverLog("Error getting Bluetooth address");
		return;
	}
	if (!startupWindowsSocket()) //initialize windows sockets
	{
		DebugDriverLog("Error Initializing windows sockets");
		return;
	}
	if (!connectToEsp32()) //initialize BT windows socket for connecting to ESP32
	{
		DebugDriverLog("Error connecting to Bluetooth device");
		return;
	}
	else {
		//If everything went fine we're connected
		m_isConnected = true;
	}
}

void BTSerialCommunicationManager::BeginListener(const std::function<void(VRCommData_t)>& callback) {
	DebugDriverLog("Begun listener");
	m_threadActive = true;
	m_serialThread = std::thread(&BTSerialCommunicationManager::ListenerThread, this, callback);
}

void BTSerialCommunicationManager::ListenerThread(const std::function<void(VRCommData_t)>& callback) {
	//DebugDriverLog("In listener thread");
	std::this_thread::sleep_for(std::chrono::milliseconds(ARDUINO_WAIT_TIME));
	PurgeBuffer();

	while (m_threadActive) {
		std::string receivedString;
		bool readSuccessful = ReceiveNextPacket(receivedString);
		
		if (readSuccessful) {
			VRCommData_t commData = m_encodingManager->Decode(receivedString);

			callback(commData);
		}
		else {
			DebugDriverLog("Detected that arduino has disconnected! Stopping listener...");
			//We should probably do more logic for trying to reconnect to the arduino
			//For now, it should be obvious to people that the arduinos have disconnected
			m_threadActive = false;
		}

	}
}

bool BTSerialCommunicationManager::ReceiveNextPacket(std::string& buff) {
	//DebugDriverLog("Waiting to recieve a message");
	char nextChar[1];
	do {
		int recievedMessageLength = 1;
		int recieveResult = recv(btClientSocket, nextChar, recievedMessageLength, 0); //if your socket is blocking, this will block until a
		if (recieveResult < 0)                                                               //a message is recieved. If not, it will return right 
		{                                                                                    //away
			continue;
		}
		buff += nextChar[0];
		//printf("Message recieved - \r\n");
		//DebugDriverLog("Length of buffer: %d, nextchar: %c", buff.length(), nextChar[0]);
	} while (nextChar[0] != '\n');
	//DebugDriverLog("Packet received! Length: %d, Packet: %s", buff.length(), buff);
	
	/*DWORD dwCommEvent;
	DWORD dwRead = 0;

	if (!SetCommMask(m_hSerial, EV_RXCHAR)) {
		DebugDriverLog("Error setting comm mask");
		return false;
	}

	char nextChar;
	int bytesRead = 0;
	if (WaitCommEvent(m_hSerial, &dwCommEvent, NULL)) {
		do {
			if (ReadFile(m_hSerial, &nextChar, 1, &dwRead, NULL)) {
				buff += nextChar;
				bytesRead++;
			}
			else {
				DebugDriverLog("Read file error");
				return false;
			}
		} while (nextChar != '\n');
	}
	else {
		DebugDriverLog("Error in comm event");
		return false;
	}

	*/return true;
}
bool BTSerialCommunicationManager::PurgeBuffer() {
	return true;//PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void BTSerialCommunicationManager::Disconnect() {
	if (m_isConnected) {
		if (m_threadActive) {
			m_threadActive = false;
			m_serialThread.join();
		}
		m_isConnected = false;
		//CloseHandle(m_hSerial);


		//Disconnect
	}
}
//May want to get a heartbeat here instead?
bool BTSerialCommunicationManager::IsConnected() {
	return m_isConnected;
}

/// <summary>
/// Gets the bluetooth devices paired with this machine and 
/// finds an ESP32. If it finds one, its BT address is stored.
/// </summary>
bool BTSerialCommunicationManager::getPairedEsp32BtAddress() {
	BLUETOOTH_DEVICE_SEARCH_PARAMS btDeviceSearchParameters =
	{
	  sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS), //size of object
	  1, //return authenticated devices
	  0, //return remembered devices
	  1, //return unknown devices
	  1, //return connected devices
	  1, //issue inquery
	  2, //timeout multipler. Multiply this value by 1.28 seconds to get timeout.
	  NULL //radio handler
	};
	BLUETOOTH_DEVICE_INFO btDeviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO),0 }; //default
	HBLUETOOTH_DEVICE_FIND btDevice = NULL;
	btDevice = BluetoothFindFirstDevice(&btDeviceSearchParameters, &btDeviceInfo); //returns first BT device connected to this machine
	if (btDevice == NULL) {
		DebugDriverLog("Could not find any bluetooth devices.");
		return false;
	}
	do {
		//wprintf(L"Checking %s.\r\n", btDeviceInfo.szName);
		if (wcsncmp(btDeviceInfo.szName, L"ESP32", 5) == 0) {
			DebugDriverLog("ESP32 found!\r\n");
			if (btDeviceInfo.fAuthenticated)  //I found that if fAuthenticated is true it means the device is paired.
			{
				DebugDriverLog("ESP32 is authenticated.\r\n");
				esp32BtAddress = btDeviceInfo.Address.ullLong;
				return true;
			}
			else {
				DebugDriverLog("This ESP32 is not authenticated. Please pair with it first.\r\n");
			}
		}
	} while (BluetoothFindNextDevice(btDevice, &btDeviceInfo)); //loop through remaining BT devices connected to this machine

	DebugDriverLog("Could not find a paired ESP32.\r\n");
	return false;
}

/// <summary>
/// Windows sockets need an initialization method called before they are used.
/// </summary>
bool BTSerialCommunicationManager::startupWindowsSocket() {
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	int wsaStartupError = WSAStartup(wVersionRequested, &wsaData); //call this before using BT windows socket.
	if (wsaStartupError != 0) {
		DebugDriverLog("WSAStartup failed with error: %d", wsaStartupError);
		return false;
	}
	return true;
}

/// <summary>
/// Sets up bluetooth socket to communicate with ESP32.
/// </summary>
bool BTSerialCommunicationManager::connectToEsp32() {
	btClientSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM); //initialize BT windows socket
	memset(&btSocketAddress, 0, sizeof(btSocketAddress));
	btSocketAddress.addressFamily = AF_BTH;
	btSocketAddress.serviceClassId = RFCOMM_PROTOCOL_UUID;
	btSocketAddress.port = 0; //port needs to be 0 if the remote device is a client. See references.
	btSocketAddress.btAddr = esp32BtAddress; //this is the BT address of the remote device.
	if (connect(btClientSocket, (SOCKADDR*)&btSocketAddress, sizeof(btSocketAddress)) != 0) //connect to the BT device.
	{
		DebugDriverLog("Could not connect socket to ESP32. Error %ld", WSAGetLastError());
		return false;
	}
	unsigned long nonBlockingMode = 1;
	if (ioctlsocket(btClientSocket, FIONBIO, (unsigned long*)&nonBlockingMode) != 0) //set the socket to be non-blocking, meaning
	{                                                                                //it will return right away when sending/recieving
		DebugDriverLog("Could not set socket to be non-blocking.");
		return false;
	}
	return true;
}

bool BTSerialCommunicationManager::sendMessageToEsp32() {
	const char* message = "Message from Windows\r\n";
	int sendResult = send(btClientSocket, message, (int)strlen(message), 0); //send your message to the BT device
	if (sendResult == SOCKET_ERROR) {
		DebugDriverLog("Sending to ESP32 failed. Error code %d", WSAGetLastError());
		closesocket(btClientSocket);
		WSACleanup();
		return false;
	}
	return true;
}