#pragma once
#include <chrono>
#include <functional>
#include <thread>

bool retry(std::function<bool()> func, short attempts, short timeout);