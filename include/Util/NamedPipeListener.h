#pragma once

#include <Windows.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>

#include "DriverLog.h"
#include "Util/Windows.h"

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
  explicit NamedPipeListener(std::string pipeName) : pipeName_(std::move(pipeName)), hPipeInst_(nullptr), threadActive_(false) {}

  ~NamedPipeListener() {
    StopListening();
  }

  bool StartListening(const std::function<void(T*)>& callback) {
    if (threadActive_.exchange(true))
      // Thread already running
      return false;

    thread_ = std::thread(&NamedPipeListener<T>::ListenerThread, this, callback);

    return true;
  }

  void StopListening() {
    if (threadActive_.exchange(false))
      // Thread running
      thread_.join();
  }

  bool IsConnected() const {
    return threadActive_;
  }

  void LogError(const char* error) const {
    DriverLog("%s (%s) - Error: %s", error, pipeName_.c_str(), GetLastErrorAsString().c_str());
  }

  void LogMessage(const char* message) const {
    DriverLog("%s (%s)", message, pipeName_.c_str());
  }

  template <typename W>
  bool Write(const W& data) {
    DWORD dwWritten = 0;

    if (!hPipeInst_) {
      LogMessage("A pipe has not been initialised and so cannot be written to");
      return false;
    }

    bool success = WriteFile(
        hPipeInst_,
        (LPCVOID)&data,
        sizeof(data),
        &numBytesWritten,
        NULL
    );

    if (!success) {
      LogError("Failed to write to named pipe");
      return false;
    };

    return true;
  }

 private:
  bool Connect(NamedPipeListenerData<T>* data) {
    if (!ConnectNamedPipe(data->hPipeInst, &data->oOverlap)) {
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

  void DisconnectAndReconnect(NamedPipeListenerData<T>* data) {
    LogMessage("Disconnecting and reconnecting named pipe");
    if (!DisconnectNamedPipe(data->hPipeInst)) LogError("Failed to disconnect");

    if (!Connect(data)) LogError("Error reconnecting to pipe from disconnect");
  }

  void ListenerThread(const std::function<void(T*)>& callback) {
    HANDLE hEvent = CreateEventA(nullptr, TRUE, TRUE, nullptr);
    if (hEvent == nullptr) {
      LogError("CreateEvent failed");
      return;
    }

    hPipeInst_ = CreateNamedPipeA(
        pipeName_.c_str(),              // pipe name
        PIPE_ACCESS_DUPLEX |            // read/write access
            FILE_FLAG_OVERLAPPED,       // overlapped mode
        PIPE_TYPE_MESSAGE |             // message-type pipe
            PIPE_READMODE_MESSAGE |     // message read mode
            PIPE_WAIT,                  // blocking mode
        PIPE_UNLIMITED_INSTANCES,       // unlimited instances
        static_cast<DWORD>(sizeof(T)),  // output buffer size
        static_cast<DWORD>(sizeof(T)),  // input buffer size
        timeoutMilli_,                  // client time-out
        nullptr);                       // default security attributes
    if (hPipeInst_ == INVALID_HANDLE_VALUE) {
      LogError("CreateNamedPipe failed");
      CloseHandle(hEvent);
      return;
    }

    NamedPipeListenerData<T> listenerData{};
    listenerData.oOverlap.hEvent = hEvent;
    listenerData.hPipeInst = hPipeInst_;

    if (!Connect(&listenerData)) return;

    LogMessage("Successfully connected to pipe");
    while (threadActive_) {
      switch (const DWORD dwWaitResult = WaitForSingleObject(listenerData.oOverlap.hEvent, timeoutMilli_)) {
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
        if (ReadFile(listenerData.hPipeInst, listenerData.chRequest, sizeof(T), &listenerData.dwBytesRead, &listenerData.oOverlap)) {
          if (listenerData.dwBytesRead > 0) {
            listenerData.fPendingIO = false;
            listenerData.state = NamedPipeListenerState::Callback;
          } else
            DisconnectAndReconnect(&listenerData);
        } else {
          if (GetLastError() == ERROR_IO_PENDING)
            listenerData.fPendingIO = true;
          else {
            LogError("Pipe received data but failed to read");
            DisconnectAndReconnect(&listenerData);
          }
        }
      } else {  // Callback (see above)
        if (listenerData.dwBytesRead == sizeof(T)) {
          callback((T*)listenerData.chRequest);
          listenerData.state = NamedPipeListenerState::Reading;
        } else
          DisconnectAndReconnect(&listenerData);
      }
    }

    CloseHandle(hPipeInst_);
    CloseHandle(hEvent);
  }

  const std::string pipeName_;
  HANDLE hPipeInst_;

  std::atomic<bool> threadActive_;
  std::thread thread_;

  unsigned long timeoutMilli_ = 5;
};