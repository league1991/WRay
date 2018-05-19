#include "stdafx.h"
#include "MemoryPool.h"


void * MemoryBlockCache::allocateBlock()
{
	Block* newBlock;
	if (m_freeList == nullptr)
	{
		newBlock = (Block*)malloc(sizeof(Block)+ m_blockDataBytes);
	}
	else
	{
		newBlock = m_freeList;
		m_freeList = newBlock->m_next;
	}

	// insert new block into usedList
	if (m_usedList)
	{
		m_usedList->m_prev = newBlock;
	}
	newBlock->m_next = m_usedList;
	newBlock->m_prev = nullptr;
	newBlock->m_dataBytes = m_blockDataBytes;
	m_usedList = newBlock;
	return (void*)(newBlock+1);
}

void MemoryBlockCache::freeBlock(void * blockData)
{
	Block* block = static_cast<Block*>(blockData) - 1;
	if (block->m_prev)
	{
		block->m_prev->m_next = block->m_next;
	}
	if (block->m_next)
	{
		block->m_next->m_prev = block->m_prev;
	}
	if (block == m_usedList)
	{
		m_usedList = block->m_next;
	}
	block->m_next = m_freeList;
	block->m_prev = nullptr;
	m_freeList = block;
}

void MemoryBlockCache::clearAllBlocks()
{
	Block* blockPtr = m_usedList;
	while (blockPtr)
	{
		Block* nextBlock = blockPtr->m_next;
		free(blockPtr);
		blockPtr = nextBlock;
	}

	blockPtr = m_freeList;
	while (blockPtr)
	{
		Block* nextBlock = blockPtr->m_next;
		free(blockPtr);
		blockPtr = nextBlock;
	}
}

MemoryPool::MemoryPool(int minBlockByte, int maxBlockByte)
{
	m_minBlockByte = (minBlockByte & (~0x1));
	m_maxBlockByte = m_minBlockByte;
	m_blockListSize = 1;
	while (m_maxBlockByte < maxBlockByte)
	{
		m_maxBlockByte *= 2;
		m_blockListSize++;
	}
	m_cache = new MemoryBlockCache[m_blockListSize];

	int blockByte = m_minBlockByte;
	for (int i = 0; i < m_blockListSize; i++)
	{
		m_cache[i].setBlockDataByte(blockByte);
		blockByte <<= 1;
	}
}
