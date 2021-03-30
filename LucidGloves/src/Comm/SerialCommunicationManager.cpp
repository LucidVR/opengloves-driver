#include <Comm/SerialCommunicationManager.h>

void SerialManager::Connect() {
	//We're not yet connected
	m_isConnected = false;

	//Try to connect to the given port throuh CreateFile
	m_hSerial = CreateFile(m_configuration.port.c_str(),
						   GENERIC_READ | GENERIC_WRITE,
						   0,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL,
						   NULL);

	if (this->m_hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {

			DebugDriverLog("Serial error: Handle was not attached. Reason: not available.");
		}
		else {
			DebugDriverLog("Serial error:Received error connecting to port");
		}
	}
	else {
		//If connected we try to set the comm parameters
		DCB dcbSerialParams = { 0 };

		//Try to get the current
		if (!GetCommState(m_hSerial, &dcbSerialParams)) {
			//If impossible, show an error
			DebugDriverLog("Serial error: failed to get current serial parameters!");
		}
		else {
			//Define serial connection parameters for the arduino board
			dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			//reset upon establishing a connection
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

			//set the parameters and check for their proper application
			if (!SetCommState(m_hSerial, &dcbSerialParams)) {
				DebugDriverLog("ALERT: Could not set Serial Port parameters");
			}
			else {
				//If everything went fine we're connected
				m_isConnected = true;
				//Flush any remaining characters in the buffers 
				PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
			}
		}
	}
}

void SerialManager::BeginListener(const std::function<void(std::string)>& callback) {
	//DebugDriverLog("Begun listener");
	m_threadActive = true;
	m_serialThread = std::thread(&SerialManager::ListenerThread, this, callback);
}

void SerialManager::ListenerThread(const std::function<void(std::string)>& callback) {
	//DebugDriverLog("In listener thread");
	std::this_thread::sleep_for(std::chrono::milliseconds(ARDUINO_WAIT_TIME));
	PurgeBuffer();
	std::string receivedString;

	while (m_threadActive) {
		bool readSuccessful = ReceiveNextPacket(receivedString);
		
		if (readSuccessful) {
			callback(receivedString);
			receivedString.clear();
		}
		else {
			DebugDriverLog("Detected that arduino has disconnected! Stopping listener...");
			//We should probably do more logic for trying to reconnect to the arduino
			//For now, it should be obvious to people that the arduinos have disconnected
			m_threadActive = false;
		}

	}
}

bool SerialManager::ReceiveNextPacket(std::string& buff) {
	DWORD dwCommEvent;
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

	return true;
}
bool SerialManager::PurgeBuffer() {
	return PurgeComm(m_hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialManager::Disconnect() {
	if (m_isConnected) {
		if (m_threadActive) {
			m_threadActive = false;
			m_serialThread.join();
		}
		m_isConnected = false;
		CloseHandle(m_hSerial);


		//Disconnect
	}
}
//May want to get a heartbeat here instead?
bool SerialManager::IsConnected() {
	return m_isConnected;
}