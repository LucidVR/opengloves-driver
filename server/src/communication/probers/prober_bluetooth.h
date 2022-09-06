#pragma once

#ifdef _WIN32
#include "prober_bluetooth_win.h"
#else
#ifdef linux
#include "prober_bluetooth_linux.h"
#endif
#endif