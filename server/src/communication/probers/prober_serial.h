#pragma once

#ifdef _WIN32
#include "prober_serial_win.h"
#else
#ifdef linux
#include "prober_serial_linux.h"
#endif
#endif