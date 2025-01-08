#pragma once
#include <functional>

void AcquireSpinLock(volatile LONG* plCount);
void AcquireSpinLock(std::function<bool()> func);
