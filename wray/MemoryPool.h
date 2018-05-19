#pragma once

class MemoryBlockCache
{
public:
	MemoryBlockCache(int blockBytes = 4) :m_blockDataBytes(blockBytes), m_usedList(nullptr), m_freeList(nullptr) {}
	~MemoryBlockCache()
	{
		clearAllBlocks();
	}

	void setBlockDataByte(int blockBytes)
	{
		m_blockDataBytes = blockBytes;
	}

	void* allocateBlock();

	void freeBlock(void* blockData);

	void clearAllBlocks();

	static int blockSize(void* data)
	{
		Block* blockPtr = static_cast<Block*>(data) - 1;
		return blockPtr->m_dataBytes;
	}
private:
	struct Block
	{
		Block* m_prev;
		Block* m_next;
		int    m_dataBytes;
	};


	int m_blockDataBytes;
	Block* m_usedList;
	Block* m_freeList;
	std::mutex m_mutex;
};

class MemoryPool
{
public:
	MemoryPool(int minBlockByte, int maxBlockByte);
	~MemoryPool()
	{
		delete[] m_cache;
	}

	void* allocate(int nBytes)
	{
		int idx = getBlockIdx(nBytes);
		return m_cache[idx].allocateBlock();
	}

	void free(void* data)
	{
		int blockSize = MemoryBlockCache::blockSize(data);
		int idx = getBlockIdx(blockSize);
		m_cache[idx].freeBlock(data);
	}

	template<typename T>
	T* allocate()
	{
		int objectSize = sizeof(T);
		void* buffer = allocate(objectSize);
		return static_cast<T*>(buffer);
	}

	template<typename T>
	void free(T* object)
	{
		object->~T();
		free((void*)object);
	}
private:
	int getBlockIdx(int nBytes)
	{
		int actualByte = m_minBlockByte;
		int idx = 0;
		while (actualByte < nBytes)
		{
			actualByte <<= 1;
			idx++;
		}

		if (actualByte > m_maxBlockByte)
		{
			return -1;
		}
		return idx;
	}

	int m_minBlockByte;
	int m_maxBlockByte;
	int m_blockListSize;
	MemoryBlockCache* m_cache;
};

