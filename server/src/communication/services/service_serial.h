#pragma once

#ifdef _WIN32
#include "service_serial_win.h"
#else
#ifdef linux
#include "service_serial_linux.h"
#endif
#endif