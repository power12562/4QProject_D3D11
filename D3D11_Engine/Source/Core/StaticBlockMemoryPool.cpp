#include "StaticBlockMemoryPool.h"
#include <intrin.h>
#include <algorithm>

StaticBlockMemoryPool::StaticBlockMemoryPool()
{
	memoryBlockSize = 0;
	memoryPageSize = 0;
	blockMaxCount = 0;
}

StaticBlockMemoryPool::~StaticBlockMemoryPool()
{
	Uninitialize();
}

bool StaticBlockMemoryPool::Initialize(size_t MemoryBlockSize, size_t MemoryPageSize)
{
	Uninitialize();

	memoryBlockSize = MemoryBlockSize;
	memoryPageSize = MemoryPageSize;

	if (MemoryPageSize % MemoryBlockSize != 0)
	{
		__debugbreak(); //블럭 크기가 맞지 않음.
		return false;
	}
	blockMaxCount = MemoryPageSize / MemoryBlockSize;

	if (!AllocateMemoryPage())
	{
		__debugbreak(); //할당 실패
		return false;
	}

	return true;
}

void StaticBlockMemoryPool::Uninitialize()
{
	std::erase_if(activeIDs, [](bool value) {return !value; });

	if (!activeIDs.empty())
		__debugbreak(); //반납 안된 메모리 존재.

	for (size_t i = 0; i < memoryPage.size(); ++i)
	{
		free(memoryPage[i]);
		memoryPage[i] = nullptr;
	}

	memoryBlockSize = 0;
	memoryPageSize = 0;
	blockMaxCount = 0;
}

void* StaticBlockMemoryPool::Allocate(size_t blockIndex)
{
	if (!GetUniqueID(blockIndex))
		return nullptr;

	size_t pageIndex = blockIndex / blockMaxCount;
	size_t realBlockIndex = blockIndex % blockMaxCount;
	while (memoryPage.size() <= pageIndex)
	{
		if (!AllocateMemoryPage())
		{
			__debugbreak(); //할당 실패
			return nullptr;
		}
	}
	char* ptr = reinterpret_cast<char*>(memoryPage[pageIndex]);
	return ptr + realBlockIndex * memoryBlockSize;
}

void StaticBlockMemoryPool::Free(size_t blockIndex)
{
	ReturnID(blockIndex);
}

bool StaticBlockMemoryPool::AllocateMemoryPage()
{
	void* newMemoryPage = malloc(memoryPageSize);
	if (newMemoryPage == nullptr)
	{
		return false;
	}
	memoryPage.push_back(newMemoryPage);
	activeIDs.reserve(blockMaxCount * memoryPage.size());
	return true;
}

bool StaticBlockMemoryPool::GetUniqueID(size_t id)
{
	if (activeIDs.size() <= id)
	{
		activeIDs.resize(id + 1);
		activeIDs[id] = true;
		return true;
	}
	else
	{
		if (activeIDs[id])
		{
			return false;
		}		
		else
		{
			activeIDs[id] = true;
			return true;
		}	    
	}
}
void StaticBlockMemoryPool::ReturnID(size_t id)
{
	activeIDs[id] = false;
}