#pragma once

#include <Comm/CommunicationManager.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <Comm/CommunicationReference.h>

#define MAX_DATA_LENGTH 100
#define ARDUINO_WAIT_TIME 1500

static constexpr int analogCap_c = 1023;

/*proposed structure for serial data
0: pinky (range 0-analog_cap)
1: ring  (range 0-analog_cap)
2: middle (range 0-analog_cap)
3: index (range 0-analog_cap)
4: thumb (range 0-analog_cap)
5: joyX (range 0-analog_cap)
6: joyY (range 0-analog_cap)
7: grab (0-1)
8: pinch (0-1)
9: buttonA (0-1)
10: buttonB (0-1)
*/

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