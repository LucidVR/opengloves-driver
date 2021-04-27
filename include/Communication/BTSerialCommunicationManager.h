#pragma once

#include "CommunicationManager.h"
#include "DeviceConfiguration.h"
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <sstream>
#include "DriverLog.h"
#include <stdlib.h>
#include <stdio.h>
#include <Winsock2.h>
#include <Ws2bth.h>
#include <BluetoothAPIs.h>

#define ARDUINO_WAIT_TIME 1000

class BTSerialCommunicationManager : public ICommunicationManager {
public:
	BTSerialCommunicationManager(std::unique_ptr<IEncodingManager> encodingManager) : m_encodingManager(std::move(encodingManager)), m_isConnected(false) {};
	//connect to the device using serial
	void Connect();
	//start a thread that listens for updates from the device and calls the callback with data
	void BeginListener(const std::function<void(VRCommData_t)>& callback);
	//returns if connected or not
	bool IsConnected();
	//close the serial port
	void Disconnect();
private:
    void ListenerThread(const std::function<void(VRCommData_t)>& callback);
    bool ReceiveNextPacket(std::string &buff);
    bool PurgeBuffer();
	bool getPairedEsp32BtAddress();
	bool startupWindowsSocket();
	bool connectToEsp32();
	bool sendMessageToEsp32();
	bool recieveMessageFromEsp32();

	bool m_isConnected;
	std::atomic<bool> m_threadActive;
	std::thread m_serialThread;

	std::unique_ptr<IEncodingManager> m_encodingManager;

	BTH_ADDR esp32BtAddress;
	SOCKADDR_BTH btSocketAddress;
	SOCKET btClientSocket;

};