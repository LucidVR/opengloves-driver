// Copyright (c) 2023 LucidVR
//
// SPDX-License-Identifier: MIT
//
// Initial Author: danwillm

#pragma once

#ifdef _WIN32
#include "service_serial_win.h"
#else
#ifdef linux
#include "service_serial_linux.h"
#endif
#endif