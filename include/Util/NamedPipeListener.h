#pragma once

#include <Windows.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>

#include "DriverLog.h"
#include "Util/Windows.h"

static const int c_namedPipeDelay = 5;

enum class NamedPipeListenerState { Connecting, Reading, Callback };

template <typename T>
struct NamedPipeListenerData {
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  bool fPendingIO;
  NamedPipeListenerState state;
  DWORD dwBytesRead;
  std::function<void(T*)> callback;
  char chRequest[sizeof(T)];
};

template <typename T>
class NamedPipeListener {
 public:
  NamedPipeListener(std::string pipeName);
  ~NamedPipeListener();

  bool StartListening(const std::function<void(T*)>& callback);
  void StopListening();
  bool IsConnected() const;
  void LogError(const char* error) const;
  void LogMessage(const char* message) const;

 private:
  bool Connect(NamedPipeListenerData<T>* data);
  void DisconnectAndReconnect(NamedPipeListenerData<T>* data);
  void ListenerThread(const std::function<void(T*)>& callback);

  const std::string _pipeName;

  std::atomic<bool> _threadActive;
  std::thread _thread;
};

template <typename T>
NamedPipeListener<T>::NamedPipeListener(std::string pipeName) : _pipeName(std::move(pipeName)), _threadActive(false) {}

template <typename T>
NamedPipeListener<T>::~NamedPipeListener() {
  StopListening();
}

template <typename T>
bool NamedPipeListener<T>::StartListening(const std::function<void(T*)>& callback) {
  if (_threadActive.exchange(true))
    // Thread already running
    return false;

  _thread = std::thread(&NamedPipeListener<T>::ListenerThread, this, callback);

  return true;
}

template <typename T>
void NamedPipeListener<T>::StopListening() {
  if (_threadActive.exchange(false))
    // Thread running
    _thread.join();
}

template <typename T>
bool NamedPipeListener<T>::Connect(NamedPipeListenerData<T>* data) {
  if (const BOOL fConnected = ConnectNamedPipe(data->hPipeInst, &data->oOverlap); !fConnected) {
    switch (GetLastError()) {
      case ERROR_IO_PENDING:
        data->fPendingIO = true;
        data->state = NamedPipeListenerState::Connecting;
        return true;

      case ERROR_PIPE_CONNECTED:
        if (SetEvent(data->oOverlap.hEvent)) {
          data->fPendingIO = false;
          data->state = NamedPipeListenerState::Reading;
          return true;
        }
        break;
    }
  }

  LogError("Failed to connect");
  data->fPendingIO = false;
  data->state = NamedPipeListenerState::Reading;

  return false;
}

template <typename T>
void NamedPipeListener<T>::DisconnectAndReconnect(NamedPipeListenerData<T>* data) {
  LogMessage("Disconnecting and reconnecting named pipe");
  if (!DisconnectNamedPipe(data->hPipeInst)) LogError("Failed to disconnect");

  if (!Connect(data)) LogError("Error reconnecting to pipe from disconnect");
}

template <typename T>
void NamedPipeListener<T>::ListenerThread(const std::function<void(T*)>& callback) {
  HANDLE hEvent = CreateEventA(nullptr, TRUE, TRUE, nullptr);
  if (hEvent == nullptr) {
    LogError("CreateEvent failed");
    return;
  }

  HANDLE hPipeInst = CreateNamedPipeA(
      _pipeName.c_str(),              // pipe name
      PIPE_ACCESS_DUPLEX |            // read/write access
          FILE_FLAG_OVERLAPPED,       // overlapped mode
      PIPE_TYPE_MESSAGE |             // message-type pipe
          PIPE_READMODE_MESSAGE |     // message read mode
          PIPE_WAIT,                  // blocking mode
      PIPE_UNLIMITED_INSTANCES,       // unlimited instances
      static_cast<DWORD>(sizeof(T)),  // output buffer size
      static_cast<DWORD>(sizeof(T)),  // input buffer size
      c_namedPipeDelay,               // client time-out
      nullptr);                       // default security attributes
  if (hPipeInst == INVALID_HANDLE_VALUE) {
    LogError("CreateNamedPipe failed");
    CloseHandle(hEvent);
    return;
  }

  NamedPipeListenerData<T> listenerData{};
  listenerData.oOverlap.hEvent = hEvent;
  listenerData.hPipeInst = hPipeInst;

  if (!Connect(&listenerData)) return;

  LogMessage("Successfully connected to pipe");
  while (_threadActive) {
    switch (const DWORD dwWaitResult = WaitForSingleObject(listenerData.oOverlap.hEvent, c_namedPipeDelay)) {
      case WAIT_OBJECT_0:
        break;
      case WAIT_TIMEOUT:
        continue;
      default:
        LogError("WaitForSingleObject failed");
        DisconnectAndReconnect(&listenerData);
        continue;
    }

    if (listenerData.fPendingIO) {
      DWORD dwBytesTransferred = 0;
      const BOOL fSuccess = GetOverlappedResult(listenerData.hPipeInst, &listenerData.oOverlap, &dwBytesTransferred, FALSE);
      if (listenerData.state == NamedPipeListenerState::Reading) {
        if (!fSuccess || dwBytesTransferred == 0) {
          LogError("GetOverlappedResult failed");
          DisconnectAndReconnect(&listenerData);
          continue;
        }
        listenerData.fPendingIO = false;
        listenerData.state = NamedPipeListenerState::Callback;
        listenerData.dwBytesRead = dwBytesTransferred;
      } else {  // Connecting/Callback/etc.
        if (!fSuccess) {
          LogError("GetOverlappedResult failed");
          break;
        }
        listenerData.state = NamedPipeListenerState::Reading;
      }
    }

    if (listenerData.state == NamedPipeListenerState::Reading) {
      if (const BOOL fSuccess =
              ReadFile(listenerData.hPipeInst, listenerData.chRequest, sizeof(T), &listenerData.dwBytesRead, &listenerData.oOverlap)) {
        if (listenerData.dwBytesRead > 0) {
          listenerData.fPendingIO = false;
          listenerData.state = NamedPipeListenerState::Callback;
        } else
          DisconnectAndReconnect(&listenerData);
      } else {
        if (GetLastError() == ERROR_IO_PENDING)
          listenerData.fPendingIO = true;
        else
          DisconnectAndReconnect(&listenerData);
      }
    } else {  // Callback (see above)
      if (listenerData.dwBytesRead == sizeof(T)) {
        callback((T*)listenerData.chRequest);
        listenerData.state = NamedPipeListenerState::Reading;
      } else
        DisconnectAndReconnect(&listenerData);
    }
  }

  CloseHandle(hPipeInst);
  CloseHandle(hEvent);
}

template <typename T>
bool NamedPipeListener<T>::IsConnected() const {
  return _threadActive;
}

template <typename T>
void NamedPipeListener<T>::LogError(const char* error) const {
  DriverLog("%s (%s) - Error: %s", error, _pipeName.c_str(), GetLastErrorAsString().c_str());
}

template <typename T>
void NamedPipeListener<T>::LogMessage(const char* message) const {
  DriverLog("%s (%s)", message, _pipeName.c_str());
}
