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

class SerialManager : public ICommunicationManager {
public:
	void Connect();
	void BeginListener(const std::function<void(VRCommData_t)>& callback);
    bool IsConnected();
    void Disconnect();
private:
    void ListenerThread(const std::function<void(VRCommData_t)>& callback);
    int ReceiveNextPacket(std::string &buff);
    bool PurgeBuffer();
	bool is_connected_;
    //Serial comm handler
    HANDLE h_serial_;
    //Connection information
    COMSTAT status_;
    //Error tracking
    DWORD errors_;
    std::atomic<bool> thread_active_;
    std::thread serial_thread_;
};

struct VRSerialConfiguration {
    char* port;

    VRSerialConfiguration(char* port) : port(port) {};
};