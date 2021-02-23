#pragma once
#include <string>

class ICommunicationManager {
public:
	virtual void Connect() = 0;
	virtual void BeginListener(void(*func)(int*)) = 0;
	virtual bool IsConnected() = 0;
	virtual void Disconnect() = 0;
	virtual ~ICommunicationManager() {};
private:
	bool is_connected_;
};
