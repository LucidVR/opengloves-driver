#pragma once
#include <string>
#include <functional>
#include <array>

class ICommunicationManager {
public:
	virtual void Connect() = 0;
	virtual void BeginListener(const std::function<void(std::string)>& callback) = 0;
	virtual bool IsConnected() = 0;
	virtual void Disconnect() = 0;
	virtual ~ICommunicationManager() {};
private:
	bool is_connected_;
};