#include "Util/Logic.h"

bool retry(std::function<bool()> func, short attempts, short timeout) {
  short retries = 0;

  do {
    if (func()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));

    retries++;
  } while (retries < attempts);

  return false;
}
