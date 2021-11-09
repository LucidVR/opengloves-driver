#pragma once
#include <functional>

bool Retry(const std::function<bool()>& func, short attempts, short timeout);