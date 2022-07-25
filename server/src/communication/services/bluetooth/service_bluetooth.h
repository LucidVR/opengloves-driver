#ifdef _WIN32
#include "service_bluetooth_win.h"
#else
#ifdef linux
#include "service_bluetooth_linux.h"
#endif
#endif