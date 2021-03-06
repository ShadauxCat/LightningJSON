#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#	include <unistd.h>
#	include <sys/mman.h>
#else
#	include <Windows.h>
#endif

namespace LightningJSON
{
	template<size_t sizeOfType>
	struct MemoryBit
	{
		void* allocType;
		MemoryBit* next;
		unsigned char data[sizeOfType];
	};

	template<size_t sizeOfType>
	class PoolAllocator
	{
	public:
		static void* alloc();
		static void* alloc(size_t count);
		static void free(void* addr);

		template<typename T>
		struct rebind
		{
			typedef PoolAllocator<sizeof(T)> otherAllocator;
		};

	private:
		static thread_local MemoryBit<sizeOfType>* ms_firstBit;

		static inline constexpr size_t max_(size_t a, size_t b)
		{
			return a < b ? b : a;
		}

		static inline constexpr size_t getPageSize_()
		{
			// TODO HACK: Assuming page size is 4096 because otherwise this gets into initialization order problems with global vars.
			return PoolAllocator::max_(4096, sizeOfType);
		}

		static constexpr size_t ms_blocksPerAlloc = PoolAllocator<sizeOfType>::max_(PoolAllocator<sizeOfType>::getPageSize_() / sizeof(MemoryBit<sizeOfType>), 32);
		static constexpr size_t ms_allocSize = ms_blocksPerAlloc * sizeof(MemoryBit<sizeOfType>);
	};

	template<size_t sizeOfType>
	thread_local MemoryBit<sizeOfType>* PoolAllocator<sizeOfType>::ms_firstBit = nullptr;

	template<size_t sizeOfType>
	void* PoolAllocator<sizeOfType>::alloc(size_t count)
	{
		void* ret = malloc(count * sizeOfType + (sizeof(intptr_t) * 2));
		memset(ret, 1, sizeof(intptr_t) * 2);
		return reinterpret_cast<unsigned char*>(ret) + (sizeof(intptr_t) * 2);
	}

	template<size_t sizeOfType>
	void* PoolAllocator<sizeOfType>::alloc()
	{
		MemoryBit<sizeOfType>* bit = ms_firstBit;

		if (bit)
		{
			ms_firstBit = bit->next;
			return bit->data;
		}

		MemoryBit<sizeOfType>* newBits = (MemoryBit<sizeOfType>*)
#ifdef _WIN32
			VirtualAlloc(nullptr, ms_allocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
			mmap(nullptr, ms_allocSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#endif
		size_t const maxBlock = ms_blocksPerAlloc - 1;
		for (size_t i = 1; i < maxBlock; ++i)
		{
			newBits[i].allocType = 0;
			newBits[i].next = &newBits[i + 1];
		}
		newBits[maxBlock].allocType = 0;
		newBits[maxBlock].next = nullptr;

		newBits[0].allocType = 0;

		ms_firstBit = &newBits[1];

		return newBits[0].data;
	}

	template<size_t sizeOfType>
	void PoolAllocator<sizeOfType>::free(void* addr)
	{
		void* allocType = reinterpret_cast<void*>(reinterpret_cast<intptr_t*>(addr)[-2]);
		if (allocType == 0)
		{
			MemoryBit<sizeOfType>* bit = reinterpret_cast<MemoryBit<sizeOfType>*>(reinterpret_cast<intptr_t*>(addr) - 2);

			bit->next = ms_firstBit;
			ms_firstBit = bit;
		}
		else
		{
			::free(reinterpret_cast<intptr_t*>(addr) - 2);
		}
	}
}
