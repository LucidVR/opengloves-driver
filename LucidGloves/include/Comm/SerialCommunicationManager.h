#pragma once

#include "Comm/CommunicationManager.h"
#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <sstream>
#include "DriverLog.h"

#define ARDUINO_WAIT_TIME 2000

static constexpr float c_maxAnalogValue = 1023;

struct VRSerialConfiguration_t {
    std::string port;

    VRSerialConfiguration_t(std::string port) : port(port) {};
};

class SerialManager : public ICommunicationManager {
public:
    SerialManager(VRSerialConfiguration_t configuration) : m_configuration(configuration) {};
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

    VRSerialConfiguration_t m_configuration;
};

