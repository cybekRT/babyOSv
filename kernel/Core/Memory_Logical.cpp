#include"Memory.hpp"

namespace Memory::Logical
{
	PageDirectory* pageDirectory = (PageDirectory*)0xFFFFF000;

	bool Init()
	{
		return true;
	}

	PageDirectory* GetLogicPageDirectory()
	{
		return (PageDirectory*)0xFFFFFC00;
	}

	PageTable* GetLogicPageTable(u16 index)
	{
		return (PageTable*)(0xFFC00000 | (index << 12));
	}

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

	void* FindFreeLogicalSpace(unsigned size)
	{
		// It's for kernel-space only~!
		size = (size + 0xFFF) & (~0xFFF);
		unsigned pagesCountNeeded = size >> 12;
		unsigned pagesCountFree = 0;

		auto abToAddr = [](unsigned a, unsigned b) -> void* {
			void* addr = (void*)((a << 22) | (b << 12));
			return addr;
		};

		unsigned startA = 512, startB = 0;
		for(unsigned a = 512; a < 1024; a++)
		{
			if(pageDirectory->entries[a].IsUsed())
			{
				PageTable* pt = (PageTable*)(0xFFC00000 | (a << 12));
				for(unsigned b = 0; b < 1024; b++)
				{
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

	void Map(void* physAddress, void* logicAddress)
	{
		if(!physAddress || !logicAddress)
		{
			PutString("=== Failed mapping memory ===");
			for(;;);
		}

		unsigned directoryIndex = ((unsigned)logicAddress) >> 22;
		unsigned tableIndex = (((unsigned)logicAddress) >> 12) & 0x3ff;

		PageDirectoryEntry* pde = (PageDirectoryEntry*)(0xFFFFF000 | (directoryIndex * 4));
		PageTable* pt = (PageTable*)(0xFFC00000 | (directoryIndex << 12));
		PageTableEntry* pte = (PageTableEntry*)(0xFFC00000 | (directoryIndex << 12) | (tableIndex * sizeof(PageDirectoryEntry)));

		if(!pde->address)
		{
			PageTable* ptPhys = (PageTable*)Physical::Alloc(sizeof(PageTable));
			pde->address = ((unsigned)ptPhys) >> 12;
			pde->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;

			// Refresh CR3
			__asm
			(
				"mov %%cr3, %%eax \n"
				"mov %%eax, %%cr3"
			: : : "eax");

			memset(pt, 0, sizeof(PageTable));
		}

		pte->address = ((unsigned)physAddress >> 12);
		pte->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;
	}

	void* Map(void* physAddress, void* logicAddress, unsigned length)
	{
		if(!logicAddress)
		{
			logicAddress = FindFreeLogicalSpace(length);
			if(!logicAddress)
				return nullptr;
		}

		length = (length + 0xFFF) >> 12;

		void* cPhysAddress = physAddress;
		void* cLogicAddress = logicAddress;
		while(length)
		{
			Map(cPhysAddress, cLogicAddress);

			//length -= 0x1000;
			length--;
			cPhysAddress = (void*)((char *)cPhysAddress + 0x1000);
			cLogicAddress = (void*)((char *)cLogicAddress + 0x1000);
		}

		return logicAddress;
	}

	void Unmap(void* logicAddress)
	{
		ASSERT( (((u32)logicAddress) & 0x3ff) == 0, "Unmapping unaligned address");

		// TODO: free page table if empty

		auto dirIndex = ((u32)logicAddress) >> 22;
		auto tableIndex = (((u32)logicAddress) >> 12) & 0x3ff;

		auto pageTable = GetLogicPageTable(dirIndex);
		auto entry = &pageTable->entries[tableIndex];

		ASSERT(entry->flags & PAGE_TABLE_FLAG_PRESENT, "Unmapping absent entry");

		entry->address = 0;
		entry->flags &= ~PAGE_TABLE_FLAG_PRESENT;

		#ifdef I386
		// Refresh cr3
		__asm(
		"mov %%cr3, %%eax \n"
		"mov %%eax, %%cr3 \n"
		: : : "eax");
		#else
		// Invalidate page
		__asm(
			"invlpg (%0)"
			: : "r"(logicAddress)
		);
		#endif
	}

	void* Alloc(unsigned allocSize)
	{
		auto PAGE_SIZE = Memory::PAGE_SIZE;
		// TODO: it doesn't need to alloc continuous memory due to the mapping...

		allocSize = (allocSize + PAGE_SIZE) & (~(PAGE_SIZE - 1));
		// Print("Logical alloc: %d\n", allocSize);

		void* logicAddr = FindFreeLogicalSpace(allocSize);
		if(!logicAddr)
		{
			PutString("Couldn't find free logical space~!");
			for(;;);

			// Physical::Free(physAddr);
			return nullptr;
		}

		// Print("Found logic addr: %p\n", logicAddr);

		for(unsigned a = 0; a < allocSize; a += PAGE_SIZE)
		{
			void* physAddr = Physical::AllocPage();
			if(!physAddr)
			{
				PutString("Not enough free memory~!");
				for(;;);

				return nullptr;
			}

			void* currentLogicAddr = (void*)(((u8*)logicAddr) + a);
			// Print("Mapping %p -> %p\n", physAddr, currentLogicAddr);
			Map(physAddr, currentLogicAddr, PAGE_SIZE);
		}

		// Print("Logical alloc returned: %p\n", logicAddr);
		return logicAddr;
	}

	void Free(void* logicAddress)
	{
		auto dirIndex = ((u32)logicAddress) >> 22;
		auto tableIndex = (((u32)logicAddress) >> 12) & 0x3ff;

		auto pageTable = GetLogicPageTable(dirIndex);
		auto entry = &pageTable->entries[tableIndex];
		auto physAddress = entry->GetAddress();

		// Print("Freeing logic: %p\n", logicAddress);
		Physical::FreePage(physAddress);
		Unmap(logicAddress);
	}
}