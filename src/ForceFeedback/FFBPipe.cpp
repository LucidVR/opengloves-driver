#include "ForceFeedback/FFBPipe.h"

#include <chrono>

#include "DriverLog.h"

std::string GetLastErrorAsString() {
  // Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();
  if (errorMessageID == 0) {
    return std::string();  // No error message has been recorded
  }

  LPSTR messageBuffer = nullptr;

  // Ask Win32 to give us the string version of that message ID.
  // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  // Copy the error message into a std::string.
  std::string message(messageBuffer, size);

  // Free the Win32's string's buffer.
  LocalFree(messageBuffer);

  return message;
}

FFBPipe::FFBPipe() : m_listenerActive(false), m_hPipe(0){};

bool FFBPipe::Start(const std::function<void(VRFFBData_t)> &callback, vr::ETrackedControllerRole handedness) {
  m_listenerActive = true;
  m_pipeThread = std::thread(&FFBPipe::PipeListenerThread, this, callback, handedness);

  return true;
}

VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap) {
  LPPIPEINST lpPipeInst;
  BOOL fWrite = FALSE;

  lpPipeInst = (LPPIPEINST)lpOverLap;

  if ((dwErr == 0) && (cbBytesRead != 0)) {
    DebugDriverLog("Received force feedback request: %d", lpPipeInst->chRequest.indexCurl);
    lpPipeInst->callback(lpPipeInst->chRequest);
  }

}

// Returns true if pending, false if the operation has completed.
bool FFBPipe::CreateAndConnectInstance(LPOVERLAPPED lpo, std::string &pipeName) {
  m_hPipe = CreateNamedPipe(pipeName.c_str(),            // pipe name
                            PIPE_ACCESS_DUPLEX |         // read/write access
                                FILE_FLAG_OVERLAPPED,    // overlapped mode
                            PIPE_TYPE_MESSAGE |          // message-type pipe
                                PIPE_READMODE_MESSAGE |  // message read mode
                                PIPE_WAIT,               // blocking mode
                            PIPE_UNLIMITED_INSTANCES,    // unlimited instances
                            sizeof(VRFFBData_t),         // output buffer size
                            sizeof(VRFFBData_t),         // input buffer size
                            5000,                        // client time-out
                            NULL);                       // default security attributes
  if (m_hPipe == INVALID_HANDLE_VALUE) {
    DriverLog("CreateNamedPipe failed with with error: %s.\n", GetLastErrorAsString().c_str());
    return 0;
  } else {
    DebugDriverLog("Created pipe successfully");
  }

  return ConnectToNewClient(lpo);
}

bool FFBPipe::ConnectToNewClient(LPOVERLAPPED lpo) {
  BOOL fConnected, fPendingIO = FALSE;

  // Start an overlapped connection for this pipe instance.
  fConnected = ConnectNamedPipe(&m_hPipe, lpo);

  // Overlapped ConnectNamedPipe should return zero.
  if (fConnected) {
    printf("ConnectNamedPipe failed with %d.\n", GetLastError());
    return 0;
  }

  switch (GetLastError()) {
      // The overlapped connection in progress.
    case ERROR_IO_PENDING:
      fPendingIO = TRUE;
      break;

      // Client is already connected, so signal an event.

    case ERROR_PIPE_CONNECTED:
      if (SetEvent(lpo->hEvent)) break;

      // If an error occurs during the connect operation...
    default: {
      printf("ConnectNamedPipe failed with %d.\n", GetLastError());
      return 0;
    }
  }
  return fPendingIO;
}

void FFBPipe::PipeListenerThread(const std::function<void(VRFFBData_t)> &callback, vr::ETrackedControllerRole handedness) {
  OVERLAPPED oConnect;
  HANDLE hConnectEvent;
  bool fPendingIO, fSuccess;
  DWORD dwWait, cbRet;

  hConnectEvent = CreateEvent(NULL,   // default security attribute
                              TRUE,   // manual reset event
                              TRUE,   // initial state = signaled
                              NULL);  // unnamed event object

  if (hConnectEvent == NULL) {
    DriverLog("CreateEvent failed with with error: %s.\n", GetLastErrorAsString().c_str());
    return;
  }

  std::string pipeName = "\\\\.\\pipe\\vrapplication\\ffb\\curl\\";
  pipeName.append((handedness == vr::ETrackedControllerRole::TrackedControllerRole_RightHand) ? "right" : "left");
  DebugDriverLog("Creating pipe: %s", pipeName.c_str());

  fPendingIO = CreateAndConnectInstance(&oConnect, pipeName);

  while (m_listenerActive) {
    dwWait = WaitForSingleObjectEx(hConnectEvent,  // event object to wait for
                                   INFINITE,       // waits indefinitely
                                   TRUE);          // alertable wait enabled
    switch (dwWait) {
        // The wait conditions are satisfied by a completed connect
        // operation.
      case 0: {
        // If an operation is pending, get the result of the
        // connect operation.

        if (fPendingIO) {
          fSuccess = GetOverlappedResult(m_hPipe,    // pipe handle
                                         &oConnect,  // OVERLAPPED structure
                                         &cbRet,     // bytes transferred
                                         FALSE);     // does not wait
          if (!fSuccess) {
            DriverLog("ConnectNamedPipe with error: %s.\n", GetLastErrorAsString().c_str());
            return;
          }
        }

        // Allocate storage for this instance.

        m_lpPipeInst = (LPPIPEINST)GlobalAlloc(GPTR, sizeof(PIPEINST));

        if (m_lpPipeInst == NULL) {
          DriverLog("GlobalAlloc failed with error: %s.\n", GetLastErrorAsString().c_str());
          return;
        }

        m_lpPipeInst->hPipeInst = m_hPipe;

        // Start the read operation for this client.
        // Note that this same routine is later used as a
        // completion routine after a write operation.

        m_lpPipeInst->cbToWrite = 0;
        m_lpPipeInst->callback = callback;
        bool fRead = ReadFileEx(m_lpPipeInst->hPipeInst, &m_lpPipeInst->chRequest, sizeof(VRFFBData_t), (LPOVERLAPPED)m_lpPipeInst,
                                (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);

        switch (GetLastError()) {
            //Disconnect if the client did so and reopen
          case ERROR_BROKEN_PIPE:
            DisconnectAndClose();
            PipeListenerThread(callback, handedness);
            break;
        }
      }

      break;

        // The wait is satisfied by a completed read or write
        // operation. This allows the system to execute the
        // completion routine.
      case WAIT_IO_COMPLETION: {
        break;
      }

      default: {
        DriverLog("WaitForSingleObjectEx with error: %s.\n", GetLastErrorAsString().c_str());
        return;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void FFBPipe::DisconnectAndClose() {
  DebugDriverLog("Closing pipe...");
  if (m_listenerActive) m_listenerActive = false;

  if (!DisconnectNamedPipe(m_lpPipeInst->hPipeInst)) {
    DebugDriverLog("DisconnectNamedPipe failed with error: %s.\n", GetLastErrorAsString().c_str());
  }

  // Close the handle to the pipe instance.
  CloseHandle(m_lpPipeInst->hPipeInst);

  // Release the storage for the pipe instance.
  if (m_lpPipeInst != NULL) GlobalFree(m_lpPipeInst);
}

void FFBPipe::Stop() {
  DriverLog("Disconnecting FFB");
  m_listenerActive = false;
  m_pipeThread.join();
  DisconnectAndClose();
}