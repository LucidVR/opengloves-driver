#pragma once

#include <Comm/CommunicationManager.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#define MAX_DATA_LENGTH 100
#define ARDUINO_WAIT_TIME 1500

class SerialManager : public ICommunicationManager {
public:
	void Connect();
	void BeginListener(const std::function<void(const float*)>& callback);
    bool IsConnected();
    void Disconnect();
private:
    void ListenerThread(const std::function<void(const float*)>& callback);
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