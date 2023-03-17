// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#include "win_util.h"

#include <Windows.h>

std::string GetLastErrorAsString() {
  const DWORD errorMessageId = ::WSAGetLastError();
  if (errorMessageId == 0) return std::string();

  LPSTR messageBuffer = nullptr;
  const size_t size = FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr,
      errorMessageId,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPSTR>(&messageBuffer),
      0,
      nullptr);

  std::string message(messageBuffer, size);

  LocalFree(messageBuffer);

  return message;
}