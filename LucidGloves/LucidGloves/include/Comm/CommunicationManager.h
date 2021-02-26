#pragma once
#include <string>
#include <functional>
#include <Comm/CommunicationReference.h>

class ICommunicationManager {
public:
	virtual void Connect() = 0;
	virtual void BeginListener(const std::function<void(VRCommData_t)>& callback) = 0;
	virtual bool IsConnected() = 0;
	virtual void Disconnect() = 0;
	virtual ~ICommunicationManager() {};
private:
	bool is_connected_;
};
