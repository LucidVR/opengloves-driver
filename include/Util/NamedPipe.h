#pragma once
#include <atomic>
#include <thread>
#include <functional>
#include <windows.h>

typedef struct {
  OVERLAPPED oOverlap;
  HANDLE hPipeInst;
  LPVOID chRequest;
  DWORD cbRead;
  DWORD cbToWrite;
  std::function<void(LPVOID)> callback;
} PIPEINST, *LPPIPEINST;

class NamedPipeUtil {
 public:
  NamedPipeUtil(std::string pipeName, size_t pipeSize);
  ~NamedPipeUtil();

  bool Start(const std::function<void(LPVOID)>& callback);
  void Stop();

 private:
  void PipeListenerThread(const std::function<void(LPVOID)>& callback);
  void ClosePipe();
  bool CreateAndConnectInstance(LPOVERLAPPED lpo, std::string& pipeName);
  bool ConnectToNewClient(LPOVERLAPPED lpo);

  std::string m_pipeName;
  size_t m_pipeSize;

  HANDLE m_hPipe;

  std::thread m_pipeThread;

  std::atomic<bool> m_listenerActive;
  std::atomic<bool> m_clientConnected;
  LPPIPEINST m_lpPipeInst;
};