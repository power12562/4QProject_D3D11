#pragma once
#include <intrin.h>
#include <memory>

namespace Utility	
{
	template<typename T>
	unsigned long SafeRelease(T& p)
	{
		if (p)
		{
			unsigned long refCount = p->Release();
			p = nullptr;
			return refCount;
		}
		return -1;
	}
	template<typename T>
	void SafeReleaseArray(T& pArr)
	{
		int i = 0;
		for (auto& p : pArr)
		{
			if (p)
			{
				p->Release();
				pArr[i] = nullptr;
				i++;
			}
		}
		return;
	}

	template<typename T>
	void SafeDelete(T& p)
	{
		if (p)
		{
			delete p;
			p = nullptr;
		}
	}
}