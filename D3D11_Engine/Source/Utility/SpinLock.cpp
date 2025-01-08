#include <Windows.h>
#include "SpinLock.h"
#include <intrin.h>

void AcquireSpinLock(volatile LONG* plCount)
{
	LONG	lOldCount;

lb_try:
	lOldCount = _InterlockedCompareExchange(plCount, 1, 0);

	if (lOldCount)
	{
		for (DWORD i = 0; i < 1024; i++)
		{
			YieldProcessor();
		}
		goto lb_try;
	}

	return;
}

void AcquireSpinLock(std::function<bool()> func)
{
lb_try:
	if (func())
	{
		for (DWORD i = 0; i < 1024; i++)
		{
			YieldProcessor();
		}
		goto lb_try;
	}
	return;
}

