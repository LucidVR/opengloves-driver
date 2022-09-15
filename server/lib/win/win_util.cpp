#include "win_util.h"

std::string GetLastErrorAsString() {
  const DWORD error_id = ::WSAGetLastError();
  if (error_id == 0) return "";

  LPSTR message_buffer = nullptr;
  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      error_id,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&message_buffer),
      0,
      nullptr);

  std::string message(message_buffer, size);

  LocalFree(message_buffer);

  return message;
}