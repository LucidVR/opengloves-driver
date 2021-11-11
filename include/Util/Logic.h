#pragma once
#include <chrono>
#include <functional>

bool Retry(const std::function<bool()>& func, short attempts, short timeout);