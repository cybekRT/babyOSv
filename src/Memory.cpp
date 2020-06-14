#include "Memory.h"

void PutString(const char* s);
void PutHex(unsigned long v);
void PutChar(char c);
extern void* kernel_end;

// TODO: stop using bubble sort everywhere! Optimmize sorting algorithms in allocator~!

void memset(void* ptr, unsigned char c, unsigned len)
{
	unsigned char* p = (unsigned char*)ptr;
	for(unsigned a = 0; a < len; a++)
	{
		p[a] = c;
	}
}

namespace Memory
{
	struct MemoryInfo_t
	{
		long long base;
		long long length;
		unsigned int type;
		unsigned int attr;
		//char padding[8];
	} __attribute__((__packed__));

	struct MemoryMap
	{
		void* address[340];
		unsigned length[340];
		bool used[340];

		MemoryMap* next;
	};

	PageDirectory* pageDirectory = (PageDirectory*)0xFFFFF000;
	MemoryInfo_t* memoryEntries = nullptr;
	unsigned memoryEntriesCount = 0;
	MemoryMap memoryMap;

	void DisableFirstMegabyteMapping()
	{
		pageDirectory->entries[0].address = 0;
		pageDirectory->entries[0].flags = 0;

		// Refresh cr3
		__asm(
		"mov %%cr3, %%eax \n"
		"mov %%eax, %%cr3 \n"
		: : : "eax");
	}

	void SortEntries()
	{
		while(true)
		{
			bool changed = false;

			for(unsigned a = 1; a < memoryEntriesCount; a++)
			{
				if(memoryEntries[a].base <= memoryEntries[a - 1].base)
				{
					auto tmp = memoryEntries[a];
					memoryEntries[a] = memoryEntries[a - 1];
					memoryEntries[a - 1] = tmp;
					changed = true;
				}
			}

			if(!changed)
				break;
		}
	}

	void FixEntries()
	{
		unsigned kEnd = ((unsigned)kernel_end) & 0x7FFFFFFF;
		for(unsigned a = 0; a < memoryEntriesCount; a++)
		{
			if(memoryEntries[a].base < (unsigned)kEnd)
			{
				unsigned diff = (unsigned)kEnd - memoryEntries[a].base;
				memoryEntries[a].base = (unsigned)kEnd;
				memoryEntries[a].length -= diff;
			}

			if(memoryEntries[a].base % 4096 || memoryEntries[a].length % 4096)
			{
				unsigned oldBase = memoryEntries[a].base;
				if(memoryEntries[a].type == 1)
					memoryEntries[a].base = (memoryEntries[a].base + 0xfff) & (~0xFFF);
				else
					memoryEntries[a].base = memoryEntries[a].base & (~0xFFF);

				unsigned diff = memoryEntries[a].base - oldBase;

				if(memoryEntries[a].type == 1)
				{
					memoryEntries[a].length -= diff;
					memoryEntries[a].length = memoryEntries[a].length & (~0xFFF);
				}
				else
				{
					memoryEntries[a].length += diff;
					memoryEntries[a].length = (memoryEntries[a].length + 0xfff) & (~0xFFF);
				}
			}
		}

		// TODO: find and fix overlapping entries
	}

	void CreateMemoryMap()
	{
		memset((void*)&memoryMap, 0, sizeof(memoryMap));

		unsigned memoryMapCount = 0;
		for(unsigned a = 0; a < memoryEntriesCount; a++)
		{
			if(memoryEntries[a].type != 1 || memoryEntries[a].length == 0)
				continue;
			if(memoryEntries[a].base > 0xffffffff)
				continue;
			if(memoryEntries[a].base + memoryEntries[a].length > 0xffffffff)
				memoryEntries[a].length = 0xffffffff - memoryEntries[a].base;

			memoryMap.address[memoryMapCount] = (void*)memoryEntries[a].base;
			memoryMap.length[memoryMapCount] = memoryEntries[a].length;

			memoryMapCount++;
		}
	}

	bool Init(void* _memoryEntries, unsigned _memoryEntriesCount)
	{
		PutString("Memory init...\n");

		memoryEntries = (MemoryInfo_t*)(((unsigned)_memoryEntries) | 0x80000000);
		memoryEntriesCount = _memoryEntriesCount;	

		DisableFirstMegabyteMapping();
		SortEntries();
		FixEntries();
		CreateMemoryMap();

		return true;
	}

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

		PutString("=== Insert memory entry failed! ===");
		for(;;);
	}

	void MergeMemoryMap()
	{
		// TODO:
	}

	void* AllocPhys(unsigned allocSize)
	{
		allocSize = (allocSize + 0xfff) & 0x1000;

		PutString("Allocating "); PutHex(allocSize); PutString(" bytes\n");
		
		void* allocatedAddress = nullptr;
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr && !allocatedAddress)
		{
			PutString("Searching...\n");
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
				PutString( currentMap->used[a] ? "u " : "F " );
				PutHex((unsigned)currentMap->address[a]);
				PutString(" - ");
				PutHex((unsigned)currentMap->length[a]);
				PutString("\n");
				if(currentMap->address[a] == 0)
					continue;

				if(!currentMap->used[a] && currentMap->length[a] >= allocSize)
				{
					allocatedAddress = currentMap->address[a];
					PutString("  Address: "); PutHex((unsigned)allocatedAddress); PutString("\n");
					
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

	void FreePhys(void* address)
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

	void Map(void* physAddress, void* logicAddress)
	{
		if(!physAddress || !logicAddress)
		{
			PutString("=== Failed mapping memory ===");
			for(;;);
		}

		PutString("Mapping "); PutHex((unsigned)physAddress); PutString(" -> "); PutHex((unsigned)logicAddress); PutString("\n");

		unsigned directoryIndex = ((unsigned)logicAddress) >> 22;
		unsigned tableIndex = (((unsigned)logicAddress) >> 12) & 0x3ff;

		PutString("Index: "); PutHex(directoryIndex); PutString(" -> "); PutHex(tableIndex); PutString("\n");

		PageDirectoryEntry* pde = (PageDirectoryEntry*)(0xFFFFF000 | (directoryIndex * 4));
		PageTable* pt = (PageTable*)(0xFFC00000 | (directoryIndex << 12));
		PageTableEntry* pte = (PageTableEntry*)(0xFFC00000 | (directoryIndex << 12) | (tableIndex * sizeof(PageDirectoryEntry)));

		if(!pde->address)
		{
			PageTable* ptPhys = (PageTable*)AllocPhys(sizeof(PageTable));
			pde->address = ((unsigned)ptPhys) >> 12;
			pde->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;

			// Refresh CR3
			__asm
			(
				"mov %%cr3, %%eax \n"
				"mov %%eax, %%cr3"
			: : : "eax");

			PutString("Memset! ");
			memset(pt, 0, sizeof(PageTable));
			PutString("Ok~!\n");
		}

		pte->address = ((unsigned)physAddress >> 12);
		pte->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;
	}

	void Map(void* physAddress, void* logicAddress, unsigned length)
	{
		length = (length + 0xFFF) >> 12;
		while(length)
		{
			Map(physAddress, logicAddress);

			//length -= 0x1000;
			length--;
			physAddress = (void*)((char *)physAddress + 0x1000);
			logicAddress = (void*)((char *)logicAddress + 0x1000);
		}
	}

	void Unmap(void* logicAddress)
	{
		// TODO: unmap
		// TODO: free page table if empty
	}

	void* FindFreeLogicalSpace(unsigned size)
	{
		PutString("FindFreeLogicalSpace\n");

		// It's for kernel-space only~!
		size = (size + 0xFFF) & (~0xFFF);
		unsigned pagesCountNeeded = size >> 12;
		unsigned pagesCountFree = 0;

		auto abToAddr = [](unsigned a, unsigned b) -> void* {
			void* addr = (void*)((a << 22) | (b << 12));
			PutString("  Found~! "); PutHex((unsigned)addr); PutString("\n");
			return addr;
		};

		unsigned startA = 512, startB = 0;
		for(unsigned a = 512; a < 1024; a++)
		{
			//if(pageDirectory->entries[a].address != 0)
			if(pageDirectory->entries[a].IsUsed())
			{
				PageTable* pt = (PageTable*)(0xFFC00000 | (a << 12));
				for(unsigned b = 0; b < 1024; b++)
				{
					//if(pt->entries[b].address == 0)
					if(!pt->entries[b].IsUsed())
					{
						pagesCountFree++;

						if(pagesCountFree >= pagesCountNeeded)
							return abToAddr(startA, startB);
					}
					else
					{
						startA = a;
						startB = b + 1;
						pagesCountFree = 0;
					}
					
				}
			}
			else
			{
				pagesCountFree += 1024;
			}
			
			if(pagesCountFree >= pagesCountNeeded)
				return abToAddr(startA, startB);
		}

		return nullptr;
	}

	void* Alloc(unsigned allocSize)
	{
		// TODO: it doesn't need to alloc continuous memory because of mapping...

		allocSize = (allocSize + 0xFFF) & (~0xFFF);

		void* physAddr = AllocPhys(allocSize);
		if(!physAddr)
		{
			PutString("Not enough free memory~!");
			for(;;);

			return nullptr;
		}

		void* logicAddr = FindFreeLogicalSpace(allocSize);

		if(!logicAddr)
		{
			PutString("Couldn't find free lofical space~!");
			for(;;);

			FreePhys(physAddr);
			return nullptr;
		}

		Map(physAddr, logicAddr, allocSize);
		return logicAddr;
	}

	void Free(void* logicAddress)
	{
		// TODO
		//FreePhys(physAddress);

		Unmap(logicAddress);
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
				
				PutString( (currentMap->used[a] ? "- " : "+ ") );
				PutHex((unsigned)currentMap->address[a]);
				PutString(" - ");
				PutHex(currentMap->length[a]);
				PutString("\n");

				if(currentMap->used[a])
					used += currentMap->length[a];
				else
					free += currentMap->length[a];
			}

			currentMap = currentMap->next;
		}

		PutString("======================\n");

		PutString("Used:  "); PutHex( used ); PutString("\n");
		PutString("Free:  "); PutHex( free ); /*PutString(" / "); PutHex( used + free );*/ PutString("\n");
		PutString("Total: "); PutHex( free + used ); PutString("\n");

		PutString("======================\n");
	}
}
