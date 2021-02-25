#include <Comm/SerialCommunicationManager.h>

void SerialManager::Connect() {
	std::cout << "connecting...." << std::endl;
	//We're not yet connected
	is_connected_ = false;
	const char* port = "COM7";

	//Try to connect to the given port throuh CreateFile
	h_serial_ = CreateFile(port,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (this->h_serial_ == INVALID_HANDLE_VALUE)
	{
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {

			printf("ERROR: Handle was not attached. Reason: %s not available.\n", port);
		}
		else
		{
			printf("Received error connecting to port");
		}
	}
	else
	{
		//If connected we try to set the comm parameters
		DCB dcbSerialParams = { 0 };

		//Try to get the current
		if (!GetCommState(h_serial_, &dcbSerialParams))
		{
			//If impossible, show an error
			printf("failed to get current serial parameters!");
		}
		else
		{
			//Define serial connection parameters for the arduino board
			dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;

			//reset upon establishing a connection
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

			//set the parameters and check for their proper application
			if (!SetCommState(h_serial_, &dcbSerialParams))
			{
				printf("ALERT: Could not set Serial Port parameters");
			}
			else
			{
				//If everything went fine we're connected
				is_connected_ = true;
				//Flush any remaining characters in the buffers 
				PurgeComm(h_serial_, PURGE_RXCLEAR | PURGE_TXCLEAR);
			}
		}
	}
}

void SerialManager::BeginListener(const std::function<void(const float*)>& callback) {
	thread_active_ = true;
	serial_thread_ = std::thread(&SerialManager::ListenerThread, this, callback);
}

void SerialManager::ListenerThread(const std::function<void(const float*)>& callback) {
	PurgeBuffer();
	std::string receivedString;

	int iteration = 0;
	while (thread_active_) {
		int bytesRead = ReceiveNextPacket(receivedString);

		//For now using base10 but will eventually switch to hex to save space
		//3FF&3FF&3FF&3FF&3FF&1&1&3FF&3FF&1&1\n is 36 chars long.
		//1023&1023&1023&1023&1023&1&1&1023&1023&1&1\n is 43 chars long.
		char* next_token;
		char* pch;
		const int stringSize = 44;
		char receivedChars[stringSize];
		strcpy_s(receivedChars, receivedString.c_str());
		int index = 0;
		float interpretedData[11] = {0,0,0,0,0,0,0,0,0,0,0};
		bool analog[11] = { true, true, true, true, true, false, false, true, true, false, false };
		pch = strtok_s(receivedChars, " &", &next_token);
		while (pch != NULL && index < 11)
		{
			float divisor = analog[index] ? 1023 : 1;
			int x = atoi(pch);
			interpretedData[index] = (float)x / divisor;
			pch = strtok_s(NULL, " &", &next_token);
			index++;
		}
		callback(interpretedData);

		receivedString.clear();
	}
}

int SerialManager::ReceiveNextPacket(std::string &buff) {
	DWORD dwCommEvent;
	DWORD dwRead = 0;

	if (!SetCommMask(h_serial_, EV_RXCHAR)) {
		std::cout << "Error setting comm mask" << std::endl;
	}

	char nextChar;
	int bytesRead = 0;

	if (WaitCommEvent(h_serial_, &dwCommEvent, NULL)) {
		do {
			if (ReadFile(h_serial_, &nextChar, 1, &dwRead, NULL))
			{
				buff += nextChar;
				bytesRead++;
			}
			else {
				std::cout << "Read file error" << std::endl;
				break;
			}
		} while (nextChar != '\n');
	}
	else {
		std::cout << "Error" << std::endl;
	}

	return bytesRead;
}
bool SerialManager::PurgeBuffer() {
	return PurgeComm(h_serial_, PURGE_RXCLEAR | PURGE_TXCLEAR);
}

void SerialManager::Disconnect() {
	if (is_connected_) {

		CloseHandle(h_serial_);

		if (thread_active_) {
			thread_active_ = false;
			serial_thread_.join();
		}
		is_connected_ = false;
		//Disconnect
	}
}
//May want to get a heartbeat here instead?
bool SerialManager::IsConnected() {
	return is_connected_;
}