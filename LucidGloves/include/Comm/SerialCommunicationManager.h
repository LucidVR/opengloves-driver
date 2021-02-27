#pragma once

#include "Comm/CommunicationManager.h"
#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <sstream>

#define MAX_DATA_LENGTH 100
#define ARDUINO_WAIT_TIME 1500

static constexpr float c_maxAnalogValue = 1023;

struct VRSerialConfiguration {
    char* port;

    VRSerialConfiguration(char* port) : port(port) {};
};

class SerialManager : public ICommunicationManager {
public:
    SerialManager(VRSerialConfiguration configuration) : m_configuration(configuration) {};
	void Connect();
	void BeginListener(const std::function<void(VRCommData_t)>& callback);
    bool IsConnected();
    void Disconnect();
private:
    void ListenerThread(const std::function<void(VRCommData_t)>& callback);
    int ReceiveNextPacket(std::string &buff);
    bool PurgeBuffer();
	bool m_isConnected;
    //Serial comm handler
    HANDLE m_hSerial;
    //Connection information
    COMSTAT m_status;
    //Error tracking
    DWORD m_errors;
    std::atomic<bool> m_threadActive;
    std::thread m_serialThread;

    VRSerialConfiguration m_configuration;
};

