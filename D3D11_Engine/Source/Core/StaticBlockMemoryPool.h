#pragma once
#include <vector>

class StaticBlockMemoryPool
{
public:
	StaticBlockMemoryPool();
	~StaticBlockMemoryPool();

	bool Initialize(size_t MemoryBlockSize, size_t MemoryPageSize = 4096);
	void Uninitialize();

	/**메모리를 요청합니다. 할당 실패하거나 이미 사용중인 Index를 접근하면 nullptr을 반환합니다. */
	void* Allocate(size_t blockIndex);

	/**메모리를 반납합니다. */
	void Free(size_t blockIndex);
private:
	size_t memoryBlockSize;
	size_t memoryPageSize;
	size_t blockMaxCount;

private:
	bool AllocateMemoryPage();
	std::vector<void*> memoryPage;

private:
	std::vector<bool> activeIDs;

private:
	bool GetUniqueID(size_t id);
	void ReturnID(size_t id);
};