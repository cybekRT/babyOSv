#include "Memory.h"
#include "bootloader_info.h"

extern const void* kernel_end;
extern const void* org_stack_beg;
extern const void* org_stack_end;
extern const u32 org_stack_size;

// TODO: stop using bubble sort everywhere! Optimmize sorting algorithms in allocator~!

void memset(void* ptr, unsigned char c, unsigned len)
{
	unsigned char* p = (unsigned char*)ptr;
	for(unsigned a = 0; a < len; a++)
	{
		p[a] = c;
	}
}

void memcpy(void* dst, void* src, u32 len)
{
	u8* _src = (u8*)src;
	u8* _dst = (u8*)dst;

	for(unsigned a = 0; a < len; a++)
		_dst[a] = _src[a];
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

	void CreateKernelStack()
	{
		void *newStackBeg = (void*)0xE0000000;
		const u32 newStackSize = 8192;

		void *stackPhys = AllocPhys(newStackSize);
		void *stackLog = Map(stackPhys, newStackBeg - newStackSize, newStackSize);

		u32 currentESP;
		u32 currentEBP;
		__asm("mov %%esp, %0" : "=r"(currentESP));
		__asm("mov %%ebp, %0" : "=r"(currentEBP));
		u32 oldStackBytesUsed = (u32)org_stack_beg - currentESP;
		u32 oldEBPOffset = (u32)org_stack_beg - currentEBP;

		u32 newESP = (u32)newStackBeg - oldStackBytesUsed;
		u32 newEBP = (u32)newStackBeg - oldEBPOffset;

		memcpy((void*)newESP, (void*)currentESP, oldStackBytesUsed);

		// Fix stack frames
		u32* _ebp = (u32*)newEBP;
		while(*_ebp)
		{
			u32 offset = (u32)org_stack_beg - *_ebp;
			*_ebp = (u32)newStackBeg - offset;
			_ebp = (u32*)*_ebp;
		}

		__asm("mov %0, %%esp" : : "r"(newESP));
		__asm("mov %0, %%ebp" : : "r"(newEBP));
	}

	bool Init()
	{
		PutString("Memory init...\n");

		memoryEntries = (MemoryInfo_t*)(((unsigned)bootloader_info_ptr->memoryEntries) | 0x80000000);
		memoryEntriesCount = *bootloader_info_ptr->memoryEntriesCount;	

		DisableFirstMegabyteMapping();
		SortEntries();
		FixEntries();
		CreateMemoryMap();

		CreateKernelStack();

		return true;
	}

	void InsertMemoryMapEntry(void* address, unsigned length)
	{
		ENTER_CRITICAL_SECTION();

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

					EXIT_CRITICAL_SECTION();
					return;
				}
			}

			currentMap = currentMap->next;
		}

		// TODO: add allocating and inserting new memory map after last one

		PutString("=== Insert memory entry failed! ===");
		for(;;);
	}

	template<typename T>
	void Swap(T &a, T &b)
	{
		T old = a;
		a = b;
		b = old;
	}

	void SortMemoryMap()
	{
		ENTER_CRITICAL_SECTION();

		PutString("Sorting... ");
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

		EXIT_CRITICAL_SECTION();
	}

	void MergeMemoryMap()
	{
		ENTER_CRITICAL_SECTION();

		SortMemoryMap();

		PutString("Merging... ");
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

							PutString("M ");
						}
					}
				}
			} while(changed);

			currentMap = currentMap->next;
		}

		PutString("Merged~!\n");
		// TODO: move entries from next memory map to previous, if there is enough space!

		EXIT_CRITICAL_SECTION();
	}

	void* AllocPhys(unsigned allocSize)
	{
		ENTER_CRITICAL_SECTION();
		allocSize = (allocSize + 0xFFF) & (~0xFFF);

		//PutString("Allocating "); PutHex(allocSize); PutString(" bytes\n");
		Print("Allocating %d bytes\n", allocSize);
		
		void* allocatedAddress = nullptr;
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr && !allocatedAddress)
		{
			//PutString("Searching...\n");
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
				//PutString( currentMap->used[a] ? "u " : "F " );
				//PutHex((unsigned)currentMap->address[a]);
				//PutString(" - ");
				//PutHex((unsigned)currentMap->length[a]);
				//PutString("\n");
				if(currentMap->address[a] == 0)
					continue;

				if(!currentMap->used[a] && currentMap->length[a] >= allocSize)
				{
					allocatedAddress = currentMap->address[a];
					//PutString("  Address: "); PutHex((unsigned)allocatedAddress); PutString("\n");
					
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

		EXIT_CRITICAL_SECTION();
		return allocatedAddress;
	}

	void FreePhys(void* address)
	{
		ENTER_CRITICAL_SECTION();

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

					EXIT_CRITICAL_SECTION();
					return;
				}
			}
		}

		PutString("=== Failed to free phys memory ===");
		for(;;);
	}

	void* FindFreeLogicalSpace(unsigned size)
	{
		ENTER_CRITICAL_SECTION();
		//PutString("FindFreeLogicalSpace\n");

		// It's for kernel-space only~!
		size = (size + 0xFFF) & (~0xFFF);
		unsigned pagesCountNeeded = size >> 12;
		unsigned pagesCountFree = 0;

		auto abToAddr = [](unsigned a, unsigned b) -> void* {
			void* addr = (void*)((a << 22) | (b << 12));
			//PutString("  Found~! "); PutHex((unsigned)addr); PutString("\n");
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
						{
							EXIT_CRITICAL_SECTION();
							return abToAddr(startA, startB);
						}
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
			{
				EXIT_CRITICAL_SECTION();
				return abToAddr(startA, startB);
			}
		}

		return nullptr;
	}

	void Map(void* physAddress, void* logicAddress)
	{
		ENTER_CRITICAL_SECTION();
		if(!physAddress || !logicAddress)
		{
			PutString("=== Failed mapping memory ===");
			for(;;);
		}

		//PutString("Mapping "); PutHex((unsigned)physAddress); PutString(" -> "); PutHex((unsigned)logicAddress); PutString("\n");

		unsigned directoryIndex = ((unsigned)logicAddress) >> 22;
		unsigned tableIndex = (((unsigned)logicAddress) >> 12) & 0x3ff;

		//PutString("Index: "); PutHex(directoryIndex); PutString(" -> "); PutHex(tableIndex); PutString("\n");

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

			//PutString("Memset! ");
			memset(pt, 0, sizeof(PageTable));
			//PutString("Ok~!\n");
		}

		pte->address = ((unsigned)physAddress >> 12);
		pte->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;

		EXIT_CRITICAL_SECTION();
	}

	void* Map(void* physAddress, void* logicAddress, unsigned length)
	{
		ENTER_CRITICAL_SECTION();
		if(!logicAddress)
		{
			logicAddress = FindFreeLogicalSpace(length);
			if(!logicAddress)
				return nullptr;
		}

		length = (length + 0xFFF) >> 12;
		//PutString("Mapping len: "); PutHex(length); PutString("\n");

		void* cPhysAddress = physAddress;
		void* cLogicAddress = logicAddress;
		while(length)
		{
			//Print("Mapping %x -> %x\n", cPhysAddress, cLogicAddress);
			Map(cPhysAddress, cLogicAddress);

			//length -= 0x1000;
			length--;
			cPhysAddress = (void*)((char *)cPhysAddress + 0x1000);
			cLogicAddress = (void*)((char *)cLogicAddress + 0x1000);
		}

		EXIT_CRITICAL_SECTION();
		return logicAddress;
	}

	PageDirectory* GetLogicPageDirectory()
	{
		return (PageDirectory*)0xFFFFFC00;
	}

	PageTable* GetLogicPageTable(u16 index)
	{
		return (PageTable*)(0xFFC00000 | (index << 12));
	}

	void Unmap(void* logicAddress)
	{
		ASSERT( (((u32)logicAddress) & 0x3ff) == 0, "Unmapping unaligned address");
		ENTER_CRITICAL_SECTION();

		// TODO: free page table if empty

		auto dirIndex = ((u32)logicAddress) >> 22;
		auto tableIndex = (((u32)logicAddress) >> 12) & 0x3ff;
		
		auto pageTable = GetLogicPageTable(dirIndex);
		auto entry = &pageTable->entries[tableIndex];
		
		ASSERT(entry->flags & PAGE_TABLE_FLAG_PRESENT, "Unmapping absent entry");

		entry->address = 0;
		entry->flags &= ~PAGE_TABLE_FLAG_PRESENT;

		// Invalidate page
		// TODO: supported since 486 :(
		__asm(
			"invlpg (%0)"
			: : "r"(logicAddress)
		);

		EXIT_CRITICAL_SECTION();
	}

	void* Alloc(unsigned allocSize)
	{
		ENTER_CRITICAL_SECTION();
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
			PutString("Couldn't find free logical space~!");
			for(;;);

			FreePhys(physAddr);
			return nullptr;
		}

		Map(physAddr, logicAddr, allocSize);

		EXIT_CRITICAL_SECTION();
		return logicAddr;
	}

	void Free(void* logicAddress)
	{
		ENTER_CRITICAL_SECTION();

		// TODO
		auto dirIndex = ((u32)logicAddress) >> 22;
		auto tableIndex = (((u32)logicAddress) >> 12) & 0x3ff;
		
		auto pageTable = GetLogicPageTable(dirIndex);
		auto entry = &pageTable->entries[tableIndex];
		auto physAddress = entry->GetAddress();

		PutString("Freeing phys memory!\n");
		FreePhys(physAddress);

		PutString("Unmapping memory: "); PutHex((u32)logicAddress); PutString("\n");
		Unmap(logicAddress);

		EXIT_CRITICAL_SECTION();
	}

	void PrintMemoryMap()
	{
		ENTER_CRITICAL_SECTION();

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
		EXIT_CRITICAL_SECTION();
	}

	struct MallocHeader
	{
		u32 length;
		MallocHeader* next;
		u8 padding[8];
	} __attribute__((packed));

	MallocHeader* mallocPage = nullptr;

	bool MallocResize(unsigned bytes)
	{
		u32 totalSize = ((bytes + sizeof(MallocHeader)) + 0xFFF) & (~0xFFF);
		auto newHeader = (MallocHeader*)Alloc(totalSize); // TODO: use pages count
		if(!newHeader)
			return false;

		newHeader->length = totalSize - sizeof(MallocHeader);
		newHeader->next = nullptr;

		if(mallocPage == nullptr)
		{
			mallocPage = newHeader;
		}
		else
		{
			auto ptr = mallocPage;
			while(ptr->next != nullptr)
				ptr = ptr->next;

			ptr->next = newHeader;
		}

		return true;
	}

	void* Malloc(unsigned bytes)
	{
		bytes = (bytes + 3) & (~3);
		
		ENTER_CRITICAL_SECTION();
		// PutString("Malloc: "); PutHex(bytes); PutString("\n");
		EXIT_CRITICAL_SECTION();

		if(mallocPage == nullptr)
		{
			PutString("Malloc resize...\n");
			if(!MallocResize(bytes))
				return nullptr;
		}

		// First fit
		auto ptr = mallocPage;
		auto prevPtr = (MallocHeader*)nullptr;
		while(ptr)
		{
			// PutString("Cur len: "); PutHex(ptr->length); PutString(" >=? "); PutHex(bytes); PutString("\n");

			if(ptr->length >= bytes)
			{
				if(ptr->length >= bytes + sizeof(MallocHeader))
				{
					// PutString("A ");

					auto oldLength = ptr->length;
					auto oldNext = ptr->next;
					auto newHeader = (MallocHeader*)(((char*)ptr) + bytes + sizeof(MallocHeader));
					ptr->length = bytes;
					ptr->next = newHeader;

					newHeader->length = oldLength - bytes - sizeof(MallocHeader);
					newHeader->next = oldNext;
				}
				else
				{
					// PutString("B ");
				}

				if(prevPtr)
					prevPtr->next = ptr->next;
				else
					mallocPage = ptr->next;

				return (void*)(ptr + 1); // Return memory after header
			}

			if(ptr->next == nullptr)
			{
				if(!MallocResize(bytes))
					return nullptr;
			}

			prevPtr = ptr;
			ptr = ptr->next;
		}

		return nullptr;
	}

	void Mfree(void* ptr)
	{
		// TODO: ...
	}
}
