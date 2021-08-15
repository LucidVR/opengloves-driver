#include "Util/NamedPipe.h"

#include <chrono>

#include "DriverLog.h"

static std::string GetLastErrorAsString() {
  // Get the error message ID, if any.
  DWORD errorMessageID = ::GetLastError();

  if (errorMessageID == 0) {
    return std::string();  // No error message has been recorded
  }

  LPSTR messageBuffer = nullptr;
  size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
};

static VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap) {
  LPPIPEINST lpPipeInst;
  BOOL fWrite = FALSE;

  lpPipeInst = (LPPIPEINST)lpOverLap;

  if ((dwErr == 0) && (cbBytesRead != 0)) {
    lpPipeInst->callback(&lpPipeInst->chRequest);
  }
};

NamedPipeUtil::NamedPipeUtil(std::string pipeName, size_t pipeSize) : m_pipeSize(pipeSize), m_pipeName(pipeName), m_listenerActive(false), m_hPipe(0), m_lpPipeInst(0){};

bool NamedPipeUtil::Start(const std::function<void(LPVOID)> &callback) {
  m_listenerActive = true;
  m_pipeThread = std::thread(&NamedPipeUtil::PipeListenerThread, this, callback);

  return true;
}

// Returns true if pending, false if the operation has completed.
bool NamedPipeUtil::CreateAndConnectInstance(LPOVERLAPPED lpo, std::string &pipeName) {
  m_hPipe = CreateNamedPipe(pipeName.c_str(),            // pipe name
                            PIPE_ACCESS_DUPLEX |         // read/write access
                                FILE_FLAG_OVERLAPPED,    // overlapped mode
                            PIPE_TYPE_MESSAGE |          // message-type pipe
                                PIPE_READMODE_MESSAGE |  // message read mode
                                PIPE_WAIT,               // blocking mode
                            PIPE_UNLIMITED_INSTANCES,    // unlimited instances
                            (DWORD)m_pipeSize,           // output buffer size
                            (DWORD)m_pipeSize,           // input buffer size
                            5000,                        // client time-out
                            NULL);                       // default security attributes
  if (m_hPipe == INVALID_HANDLE_VALUE) {
    DriverLog("CreateNamedPipe failed with with error: %s.\n", GetLastErrorAsString().c_str());
    return 0;
  } else {
    DriverLog("Pipe created successfully: %s", m_pipeName.c_str());
  }

  return ConnectToNewClient(lpo);
}

bool NamedPipeUtil::ConnectToNewClient(LPOVERLAPPED lpo) {
  BOOL fConnected, fPendingIO = FALSE;

  // Start an overlapped connection for this pipe instance.
  fConnected = ConnectNamedPipe(&m_hPipe, lpo);

  if (fConnected) {
    DriverLog("ConnectNamedPipe failed with %c.\n", GetLastErrorAsString().c_str());
    return 0;
  }

  switch (GetLastError()) {
    case ERROR_IO_PENDING:
      fPendingIO = TRUE;
      break;

    case ERROR_PIPE_CONNECTED:
      if (SetEvent(lpo->hEvent)) break;
      // fall through
    default: {
      DriverLog("ConnectNamedPipe failed with: %s", GetLastErrorAsString().c_str());
      return 0;
    }
  }
  return fPendingIO;
}

void NamedPipeUtil::PipeListenerThread(const std::function<void(LPVOID)> &callback) {
  OVERLAPPED oConnect{};
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

  oConnect.hEvent = hConnectEvent;
  DriverLog("Creating pipe: %s", m_pipeName.c_str());

  fPendingIO = CreateAndConnectInstance(&oConnect, m_pipeName);

  while (m_listenerActive) {
    dwWait = WaitForSingleObjectEx(hConnectEvent,  // event object to wait for
                                   INFINITE,       // waits indefinitely
                                   TRUE);          // alertable wait enabled
    switch (dwWait) {
      case 0: {
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

        m_lpPipeInst = (LPPIPEINST)GlobalAlloc(GPTR, sizeof(PIPEINST));

        if (m_lpPipeInst == NULL) {
          DriverLog("GlobalAlloc failed with error: %s.\n", GetLastErrorAsString().c_str());
          return;
        }

        m_lpPipeInst->hPipeInst = m_hPipe;

        m_lpPipeInst->cbToWrite = 0;
        m_lpPipeInst->callback = callback;

        bool fRead =
            ReadFileEx(m_lpPipeInst->hPipeInst, &m_lpPipeInst->chRequest, (DWORD)m_pipeSize, (LPOVERLAPPED)m_lpPipeInst, (LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);
        if (fRead) break;

        switch (GetLastError()) {
          case ERROR_BROKEN_PIPE:
            DriverLog("Detected that a client disconnected for pipe: %s", m_pipeName.c_str());
            ClosePipe();
            PipeListenerThread(callback);
            break;
        }
        break;
      }
      case WAIT_IO_COMPLETION: {
        break;
      }
      default: {
        DriverLog("WaitForSingleObjectEx with error: %s.\n", GetLastErrorAsString().c_str());
        return;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

void NamedPipeUtil::ClosePipe() {
  DriverLog("Closing pipe: %s", m_pipeName.c_str());
  if (!DisconnectNamedPipe(m_lpPipeInst->hPipeInst)) {
    DriverLog("DisconnectNamedPipe failed with error: %s.\n", GetLastErrorAsString().c_str());
  }

  CloseHandle(m_lpPipeInst->hPipeInst);

  // Release the storage for the pipe instance.
  if (m_lpPipeInst != NULL) GlobalFree(m_lpPipeInst);
}

void NamedPipeUtil::Stop() {
  if (m_listenerActive) {
    m_listenerActive = false;
    m_pipeThread.join();
    ClosePipe();
  }
}

NamedPipeUtil::~NamedPipeUtil() { Stop(); };