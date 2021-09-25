#pragma once
#include <functional>
#include <chrono>
#include <thread>

bool retry(std::function<bool()> func, short attempts, short timeout);