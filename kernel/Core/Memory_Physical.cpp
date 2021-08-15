#include"Memory.h"

namespace Memory::Physical
{
	MemoryMap memoryMap;

	void InsertMemoryMapEntry(void* address, unsigned length)
	{
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr)
		{
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
				if(currentMap->address[a] == 0)
				{
					currentMap->address[a] = address;
					currentMap->length[a] = length;
					currentMap->used[a] = true;

					return;
				}
			}

			currentMap = currentMap->next;
		}

		// TODO: add allocating and inserting new memory map after last one

		//PrintMemoryMap();
		PutString("=== Insert memory entry failed! ===");
		for(;;);
	}

	void SortMemoryMap()
	{
		// TODO:
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr)
		{
			bool changed;

			do
			{
				changed = false;
				for(unsigned a = 1; a < sizeof(currentMap->used); a++)
				{
					if(currentMap->address[a] == 0)
						continue;

					if(currentMap->address[a] < currentMap->address[a - 1])
					{
						auto oldAddr = currentMap->address[a - 1];
						currentMap->address[a - 1] = currentMap->address[a];
						currentMap->address[a] = oldAddr;

						auto oldUsed = currentMap->used[a - 1];
						currentMap->used[a - 1] = currentMap->used[a];
						currentMap->used[a] = oldUsed;

						auto oldLen = currentMap->length[a - 1];
						currentMap->length[a - 1] = currentMap->length[a];
						currentMap->length[a] = oldLen;

						//Swap(currentMap->address[a - 1]. currentMap->address[a]);
					}
				}
			} while(changed);

			currentMap = currentMap->next;
		}
	}

	void MergeMemoryMap()
	{
		SortMemoryMap();

		// TODO:
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr)
		{
			bool changed;

			do
			{
				changed = false;
				for(unsigned a = 1; a < sizeof(currentMap->used); a++)
				{
					if(currentMap->address[a - 1] == 0 || currentMap->address[a] == 0)
						continue;

					if(!currentMap->used[a - 1] && !currentMap->used[a])
					{
						u32 addr1 = (u32)currentMap->address[a - 1];
						u32 len1 = currentMap->length[a - 1];
						u32 addr2 = (u32)currentMap->address[a];
						u32 len2 = currentMap->length[a];

						if(addr1 + len1 == addr2)
						{
							changed = true;
							currentMap->length[a - 1] += len2;

							currentMap->address[a] = 0;
							currentMap->length[a] = 0;
						}
					}
				}
			} while(changed);

			currentMap = currentMap->next;
		}

		// TODO: move entries from next memory map to previous, if there is enough space!
	}

	void PrintMemoryMap()
	{
		PutString("\n");
		PutString("===== Memory map =====\n");

		unsigned used = 0;
		unsigned free = 0;

		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr)
		{
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
				if(currentMap->address[a] == 0)
					continue;
				
				Print("%c %p - %u\n", (currentMap->used[a] ? '-' : '+'), 
							currentMap->address[a], 
							currentMap->length[a]);

				if(currentMap->used[a])
					used += currentMap->length[a];
				else
					free += currentMap->length[a];
			}

			currentMap = currentMap->next;
		}

		PutString("======================\n");
		Print("Used:  %u bytes\nFree:  %u bytes\nTotal: %u bytes\n", used, free, free + used);
		PutString("======================\n");
	}

	void* Alloc(unsigned allocSize)
	{
		allocSize = (allocSize + 0xFFF) & (~0xFFF);

		void* allocatedAddress = nullptr;
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr && !allocatedAddress)
		{
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
				if(currentMap->address[a] == 0)
					continue;

				if(!currentMap->used[a] && currentMap->length[a] >= allocSize)
				{
					allocatedAddress = currentMap->address[a];
					
					currentMap->length[a] -= allocSize;
					currentMap->address[a] = (void*)((unsigned)currentMap->address[a] + allocSize);
					if(currentMap->length[a] == 0)
						currentMap->address[a] = 0;
					InsertMemoryMapEntry(allocatedAddress, allocSize);

					break;
				}
			}

			currentMap = currentMap->next;
		}

		// TODO: zeroing memory?
		return allocatedAddress;
	}

	void Free(void* address)
	{
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr)
		{
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
				if(currentMap->address[a] == address)
				{
					currentMap->used[a] = false;

					// TODO: maybe some counter, not to merge everytime
					MergeMemoryMap();

					return;
				}
			}
		}

		PutString("=== Failed to free phys memory ===");
		for(;;);
	}
}