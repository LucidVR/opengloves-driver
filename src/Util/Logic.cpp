#include "Util/Logic.h"

#include <thread>

bool Retry(const std::function<bool()>& func, const short attempts, const short timeout) {
  short retries = 0;

  do {
    if (func()) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout));

    retries++;
  } while (retries < attempts);

  return false;
}
