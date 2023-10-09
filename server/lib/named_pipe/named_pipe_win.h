// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include <Windows.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "opengloves_interface.h"
#include "win/win_util.h"

enum class NamedPipeListenerState { Connecting, Reading, Callback };

class INamedPipeListener {
 public:
  virtual bool StartListening() = 0;

  virtual ~INamedPipeListener() = default;
};

template <typename T>
struct NamedPipeListenerData {
  OVERLAPPED overlap;
  HANDLE handle;
  bool pending_io;
  NamedPipeListenerState state;
  DWORD bytes_read;
  std::function<void(T*)> callback;
  char request[sizeof(T)];
};

enum class NamedPipeListenerEventType { Invalid, ClientConnected };

struct NamedPipeListenerEvent {
  NamedPipeListenerEventType type;
};

template <typename T>
class NamedPipeListener : public INamedPipeListener {
 public:
  NamedPipeListener(
      std::string pipe_name, std::function<void(const NamedPipeListenerEvent& event)> on_event_callback, std::function<void(T*)> on_data_callback)
      : pipe_name_(std::move(pipe_name)),
        on_event_callback_(std::move(on_event_callback)),
        on_data_callback_(std::move(on_data_callback)),
        thread_active_(false){};

  bool StartListening() override {
    if (thread_active_.exchange(true))
      // Thread already running
      return false;

    thread_ = std::thread(&NamedPipeListener<T>::ListenerThread, this);

    return true;
  }

  bool IsConnected() const {
    return thread_active_;
  }

  void LogError(const char* error) const {
    static og::Logger& logger = og::Logger::GetInstance();

    logger.Log(og::LoggerLevel::kLoggerLevel_Error, "%s (%s) - Error: %s", error, pipe_name_.c_str(), GetLastErrorAsString().c_str());
  }

  void LogMessage(const char* message) const {
    static og::Logger& logger = og::Logger::GetInstance();

    logger.Log(og::LoggerLevel::kLoggerLevel_Info, "%s (%s)", message, pipe_name_.c_str());
  }

  ~NamedPipeListener() {
    if (thread_active_.exchange(false) && thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  bool Connect(NamedPipeListenerData<T>* data) {
    if (!ConnectNamedPipe(data->handle, &data->overlap)) {
      switch (GetLastError()) {
        case ERROR_IO_PENDING:
          data->pending_io = true;
          data->state = NamedPipeListenerState::Connecting;
          return true;

        case ERROR_PIPE_CONNECTED:
          if (SetEvent(data->overlap.hEvent)) {
            data->pending_io = false;
            data->state = NamedPipeListenerState::Reading;
            return true;
          }
          break;
      }
    }

    LogError("Failed to connect");
    data->pending_io = false;
    data->state = NamedPipeListenerState::Reading;

    return false;
  }

  void DisconnectAndReconnect(NamedPipeListenerData<T>* data) {
    LogMessage("Disconnecting and reconnecting named pipe");
    if (!DisconnectNamedPipe(data->handle)) LogError("Failed to disconnect");

    if (!Connect(data)) LogError("Error reconnecting to pipe from disconnect");
  }

  void ListenerThread() {
    HANDLE hEvent = CreateEventA(nullptr, TRUE, TRUE, nullptr);
    if (hEvent == nullptr) {
      LogError("CreateEvent failed");
      return;
    }

    HANDLE hPipeInst = CreateNamedPipeA(
        pipe_name_.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        static_cast<DWORD>(sizeof(T)),
        static_cast<DWORD>(sizeof(T)),
        5,
        nullptr);

    if (hPipeInst == INVALID_HANDLE_VALUE) {
      LogError("CreateNamedPipe failed");
      CloseHandle(hEvent);
      return;
    }

    NamedPipeListenerData<T> listener_data{};
    listener_data.overlap.hEvent = hEvent;
    listener_data.handle = hPipeInst;

    if (!Connect(&listener_data)) return;

    while (thread_active_) {
      switch (WaitForSingleObject(listener_data.overlap.hEvent, 5)) {
        case WAIT_OBJECT_0:
          break;
        case WAIT_TIMEOUT:
          continue;
        default:
          LogError("WaitForSingleObject failed");
          DisconnectAndReconnect(&listener_data);
          continue;
      }

      if (listener_data.pending_io) {
        DWORD bytes_transferred = 0;
        const BOOL success = GetOverlappedResult(listener_data.handle, &listener_data.overlap, &bytes_transferred, FALSE);
        if (listener_data.state == NamedPipeListenerState::Reading) {
          if (!success || bytes_transferred == 0) {
            LogError("GetOverlappedResult failed");
            DisconnectAndReconnect(&listener_data);
            continue;
          }
          listener_data.pending_io = false;
          listener_data.state = NamedPipeListenerState::Callback;
          listener_data.bytes_read = bytes_transferred;
        } else {  // Connecting/Callback/etc.
          if (!success) {
            LogError("GetOverlappedResult failed");
            break;
          }
          
          if (listener_data.state == NamedPipeListenerState::Connecting) {
            if (thread_active_) {
              on_event_callback_({NamedPipeListenerEventType::ClientConnected});
            }
          }
          
          listener_data.state = NamedPipeListenerState::Reading;
        }
      }

      if (listener_data.state == NamedPipeListenerState::Reading) {
        if (ReadFile(listener_data.handle, listener_data.request, sizeof(T), &listener_data.bytes_read, &listener_data.overlap)) {
          if (listener_data.bytes_read > 0) {
            listener_data.pending_io = false;
            listener_data.state = NamedPipeListenerState::Callback;
          } else
            DisconnectAndReconnect(&listener_data);
        } else {
          if (GetLastError() == ERROR_IO_PENDING)
            listener_data.pending_io = true;
          else {
            LogError("Pipe received data but failed to read");
            DisconnectAndReconnect(&listener_data);
          }
        }
      } else {  // Callback (see above)
        if (listener_data.bytes_read == sizeof(T)) {
          on_data_callback_((T*)listener_data.request);
          listener_data.state = NamedPipeListenerState::Reading;
        } else
          DisconnectAndReconnect(&listener_data);
      }
    }

    DisconnectNamedPipe(hPipeInst); // Not 100% sure this is needed, but winapi seems to like this better
    CloseHandle(hPipeInst);
    CloseHandle(hEvent);
  }

  std::string pipe_name_;

  std::atomic<bool> thread_active_;
  std::thread thread_;

  std::function<void(NamedPipeListenerEvent)> on_event_callback_;
  std::function<void(T*)> on_data_callback_;
};
