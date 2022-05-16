#include"Memory.hpp"

namespace Memory::Physical
{
	constexpr u32 MAX_MEMORY_SIZE = 32;
	/**
	 * 0 - reserved
	 * 1 - free
	 */
	u8 memoryBitmap[MAX_MEMORY_SIZE * 1024 * 1024 / PAGE_SIZE / 8];
	u32 maxMemoryIndex = 0;

	bool Init()
	{
		for(unsigned a = 0; a < sizeof(memoryBitmap); a++)
			memoryBitmap[a] = 0;
	}

	void AddFreeMemory(void* _address, u32 _length)
	{
		if((u32)_address + _length > maxMemoryIndex)
			maxMemoryIndex = ((u32)_address + _length) / PAGE_SIZE;

		u32 address = (u32)_address;
		u32 pagesCount = _length / PAGE_SIZE;
		if(!pagesCount)
			return;
		u32 index = address / PAGE_SIZE;

		if(address & (PAGE_SIZE - 1))
		{
			index++;
			pagesCount--;
			if(!pagesCount)
				return;
		}

		for(unsigned a = 0; a < pagesCount; a++)
		{
			u32 indexCurrent = index + a;

			if(indexCurrent / 8 > sizeof(memoryBitmap))
				break;

			memoryBitmap[indexCurrent / 8] |= (1 << (indexCurrent % 8));
		}
	}

	void ReserveMemory(void* _address, u32 _length)
	{
		if( ((u32)_address + _length) / PAGE_SIZE > maxMemoryIndex)
			maxMemoryIndex = ((u32)_address + _length) / PAGE_SIZE;

		u32 address = (u32)_address;
		u32 pagesCount = (_length + PAGE_SIZE - 1) / PAGE_SIZE;
		u32 index = address / PAGE_SIZE;

		for(unsigned a = 0; a < pagesCount; a++)
		{
			u32 indexCurrent = index + a;

			if(indexCurrent / 8 > sizeof(memoryBitmap))
				break;

			memoryBitmap[indexCurrent / 8] &= ~(1 << (indexCurrent % 8));
		}
	}

	void PrintMemoryMap()
	{
		PutString("\n");
		PutString("===== Memory map =====\n");

		unsigned usedCount = 0;
		unsigned freeCount = 0;

		u32 lastIndex = 0;
		u32 lastType = 0;
		for(unsigned a = 0; a < sizeof(memoryBitmap); a++)
		{
			for(unsigned b = 0; b < 8; b++)
			{
				u32 index = a * 8 + b;
				u32 currentType = !!(memoryBitmap[a] & (1 << b));

				if(currentType)
					freeCount++;
				else
					usedCount++;

				if(currentType != lastType || (a == sizeof(memoryBitmap) - 1 && b == 7) || index == maxMemoryIndex)
				{
					Print("%c %p - %u  \n", (lastType ? '+' : '-'),
								lastIndex * PAGE_SIZE,
								(index - lastIndex) * PAGE_SIZE);

					lastIndex = index;
					lastType = currentType;
				}

				if(index >= maxMemoryIndex)
					break;
			}

			if(a * 8 >= maxMemoryIndex)
				break;
		}

		PutString("======================\n");
		Print("Used:  %u bytes\nFree:  %u bytes\nTotal: %u bytes\n",
			usedCount * PAGE_SIZE, freeCount * PAGE_SIZE, (freeCount + usedCount) * PAGE_SIZE);
		PutString("======================\n");
	}

	// void* Alloc(unsigned allocSize)
	// {
	// 	allocSize = (allocSize + 0xFFF) & (~0xFFF);

	// 	void* allocatedAddress = nullptr;
	// 	MemoryMap* currentMap = &memoryMap;
	// 	while(currentMap != nullptr && !allocatedAddress)
	// 	{
	// 		for(unsigned a = 0; a < sizeof(currentMap->used); a++)
	// 		{
	// 			if(currentMap->address[a] == 0)
	// 				continue;

	// 			if(!currentMap->used[a] && currentMap->length[a] >= allocSize)
	// 			{
	// 				allocatedAddress = currentMap->address[a];

	// 				currentMap->length[a] -= allocSize;
	// 				currentMap->address[a] = (void*)((unsigned)currentMap->address[a] + allocSize);
	// 				if(currentMap->length[a] == 0)
	// 					currentMap->address[a] = 0;
	// 				InsertMemoryMapEntry(allocatedAddress, allocSize);

	// 				break;
	// 			}
	// 		}

	// 		currentMap = currentMap->next;
	// 	}

	// 	// TODO: zeroing memory?
	// 	return allocatedAddress;
	// }

	// void Free(void* address)
	// {
	// 	MemoryMap* currentMap = &memoryMap;
	// 	while(currentMap != nullptr)
	// 	{
	// 		for(unsigned a = 0; a < sizeof(currentMap->used); a++)
	// 		{
	// 			if(currentMap->address[a] == address)
	// 			{
	// 				currentMap->used[a] = false;

	// 				// TODO: maybe some counter, not to merge everytime
	// 				MergeMemoryMap();

	// 				return;
	// 			}
	// 		}
	// 	}

	// 	PutString("=== Failed to free phys memory ===");
	// 	for(;;);
	// }

	void* Alloc(unsigned allocSize)
	{
		auto PAGE_SIZE = Memory::PAGE_SIZE;
		u32 pagesCount = (allocSize + (PAGE_SIZE - 1)) / PAGE_SIZE;

		u32 lastIndex = 0;
		for(unsigned a = 0; a < sizeof(memoryBitmap); a++)
		{
			for(unsigned b = 0; b < 8; b++)
			{
				u32 index = (a * 8 + b);
				if(memoryBitmap[a] & (1 << b))
				{
					if(index - lastIndex + 1 >= pagesCount)
					{
						for(unsigned c = lastIndex; c <= index; c++)
							memoryBitmap[c / 8] &= ~(1 << (c % 8));

						Print("Physical alloc: %p\n", (index * PAGE_SIZE));
						return (void*)(index * PAGE_SIZE);
					}
				}
				else
				{
					lastIndex = index + 1;
				}
			}
		}

		FAIL("No memory...\n");
		return nullptr;
	}

	void* AllocPage()
	{
		for(unsigned a = 0; a < sizeof(memoryBitmap); a++)
		{
			for(unsigned b = 0; b < 8; b++)
			{
				if(memoryBitmap[a] & (1 << b))
				{
					memoryBitmap[a] &= ~(1 << b);
					u32 index = (a * 8 + b);

					// Print("Physical alloc-page: %p\n", (index * PAGE_SIZE));
					return (void*)(index * PAGE_SIZE);
				}
			}
		}

		FAIL("No memory...");
		return nullptr;
	}

	void FreePage(void* addr)
	{
		ASSERT(((u32)addr & (PAGE_SIZE - 1)) == 0, "Page address is not aligned~!");

		u32 index =	((u32)addr) / PAGE_SIZE;
		// Print("Physical free-page: %p\n", addr);
		memoryBitmap[index / 8] |= (1 << (index % 8));
	}
}