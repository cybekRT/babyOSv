#include "Memory.h"

void PutString(const char* s);
void PutHex(unsigned long v);
void PutChar(char c);
extern void* kernel_end;

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
		//unsigned count;
		void* address[340];
		unsigned length[340];
		bool used[340];

		MemoryMap* next;
	};

	PageDirectory* pageDirectory = nullptr;
	MemoryInfo_t* memoryEntries = nullptr;
	unsigned memoryEntriesCount = 0;
	MemoryMap memoryMap;

	void DisableFirstMegabyteMapping()
	{
		// Disable first megabyte
		//PageDirectory* pd = (PageDirectory*)pageDirectory;

		//unsigned* xxx = (unsigned*)0x1000;
		//*xxx = 0;

//__asm("xchg %%bx, %%bx\n" : : : "bx");

		PutString("===\n");
		//PutHex((unsigned)pageDirectory); PutString("\n");
		//PutHex((unsigned)pageDirectory->entries[0].address); PutString("\n");
		//PutHex((unsigned)pageDirectory->entries[0].flags); PutString("\n");

		pageDirectory->entries[0].address = 0;
		pageDirectory->entries[0].flags = 0;

		PutHex((unsigned) (&pageDirectory->entries[0]) );

		//PutHex((unsigned)pageDirectory->entries[0].address); PutString("\n");
		//PutHex((unsigned)pageDirectory->entries[0].flags); PutString("\n");
		//for(;;);

		// Refresh cr3
		__asm(
		"mov %%cr3, %%eax \n"
		"mov %%eax, %%cr3 \n"
		: : : "eax");

//		PutString("##### ");
//
//		PageTable* pt = (PageTable*)(((unsigned)pageDirectory->entries[0].address) << 12);
//		pt = (PageTable*)((unsigned)pt | 0x80000000);
//		PutHex((unsigned)pt);
//		PutString(" #####");
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
		unsigned len = 0;
		unsigned kEnd = ((unsigned)kernel_end) & 0x7FFFFFFF;
		for(unsigned a = 0; a < memoryEntriesCount; a++)
		{
			//PutString("\n--------\n");

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
					memoryEntries[a].base = (memoryEntries[a].base + 0xfff) & 0x1000;
				else
					memoryEntries[a].base = memoryEntries[a].base & 0x1000;

				unsigned diff = memoryEntries[a].base - oldBase;

				if(memoryEntries[a].type == 1)
				{
					memoryEntries[a].length -= diff;
					memoryEntries[a].length = memoryEntries[a].length & 0x1000;
				}
				else
				{
					memoryEntries[a].length += diff;
					memoryEntries[a].length = (memoryEntries[a].length + 0xfff) & 0x1000;
				}
			}

			// PutHex(memoryEntries[a].base); PutChar('\n');
			// PutHex(memoryEntries[a].length); PutChar('\n');
			// len += memoryEntries[a].length;
			// PutHex(memoryEntries[a].type); PutChar('\n');
		}

		// Find and fix overlapping entries

		//PutString("... ");
		//PutHex(len);
	}

	void CreateMemoryMap()
	{
		PutString(" "); PutHex(sizeof(MemoryMap));
		PutString("\n.....\n");

		memset((void*)&memoryMap, 0, sizeof(memoryMap));

		for(unsigned a = 0; a < memoryEntriesCount; a++)
		{
			if(memoryEntries[a].type != 1 || memoryEntries[a].length == 0)
				continue;
			if(memoryEntries[a].base > 0xffffffff)
				continue;
			if(memoryEntries[a].base + memoryEntries[a].length > 0xffffffff)
				memoryEntries[a].length = 0xffffffff - memoryEntries[a].base;

			memoryMap.address[a] = (void*)memoryEntries[a].base;
			memoryMap.length[a] = memoryEntries[a].length;
			//memoryMap.used[a] = false;
			//memoryMap.next = NULL;
		}
	}

	bool Init(void* _pageDirectory, void* _memoryEntries, unsigned _memoryEntriesCount)
	{
		PutString("Memory init...\n");

		//__asm("xchg %%bx, %%bx\n" : : : "bx");

		pageDirectory = (PageDirectory*)(unsigned(_pageDirectory) | 0x80000000);

		//PutHex((unsigned)_pageDirectory); for(;;);

		memoryEntries = (MemoryInfo_t*)(((unsigned)_memoryEntries) | 0x80000000);
		memoryEntriesCount = _memoryEntriesCount;	

		PutString("= DIS =\n");
		DisableFirstMegabyteMapping();
		PutString("= /DIS =\n");

		SortEntries();
		FixEntries();
		CreateMemoryMap();

		return true;
	}

	void Quit()
	{
		// LOL, never happens...
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

		PutString("=== Insert memory entry failed! ===");
		for(;;);
	}

	void* AllocPhys(unsigned allocSize)
	{
		allocSize = (allocSize + 0xfff) & 0x1000;

		PutString("Allocating "); PutHex(allocSize); PutString(" bytes\n");
		
		void* allocatedAddress = nullptr;
		MemoryMap* currentMap = &memoryMap;
		while(currentMap != nullptr && !allocatedAddress)
		{
			for(unsigned a = 0; a < sizeof(currentMap->used); a++)
			{
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
					return;
				}
			}
		}

		PutString("=== Failed to free phys memory ===");
		for(;;);
	}

	void MapPhys(void* physAddress, void* logicAddress)
	{
		if(!physAddress || !logicAddress)
		{
			PutString("=== Failed mapping memory ===");
			for(;;);
		}

		PutString("Mapping "); PutHex((unsigned)physAddress); PutString(" -> "); PutHex((unsigned)logicAddress); PutString("\n");

		//PageDirectory* pd = pageDirectory;
		
		unsigned directoryIndex = ((unsigned)logicAddress) >> 22;
		unsigned tableIndex = (((unsigned)logicAddress) >> 12) & 0x3ff;

		PutString("Index: "); PutHex(directoryIndex); PutString(" -> "); PutHex(tableIndex); PutString("\n");

		PageDirectoryEntry* pde = (PageDirectoryEntry*)(0xFFFFF000 | directoryIndex);
		PageTable* pt = (PageTable*)(0xFFC00000 | (directoryIndex << 12));
		PageTableEntry* pte = (PageTableEntry*)(0xFFC00000 | (directoryIndex << 12) | (tableIndex * sizeof(PageDirectoryEntry)));

		//PageTable* ptPhys = (PageTable*)((unsigned)pd->entries[directoryIndex].address << 12);
		//PageTable* ptLogic = (PageTable*)(0xFFC00000 | (directoryIndex << 12));
		//if(!ptPhys)
		if(!pde->address)
		{
			PageTable* ptPhys = (PageTable*)AllocPhys(sizeof(PageTable));
			//memset(pt, 0, sizeof(PageTable));
			//pd[directoryIndex] = pt;

			PutString("OK");
//__asm("xchg %%bx, %%bx\n" : : : "bx");
//memset(pt, 0, 4096);
			//unsigned* x = (unsigned*)pt;
			//*x = 0;
			//for(;;);

			pde->address = ((unsigned)ptPhys) >> 12;
			pde->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;

			// Refresh CR3
			__asm
			(
				"mov %%cr3, %%eax \n"
				"mov %%eax, %%cr3"
			: : : "eax");

			//unsigned* ptr = (unsigned*)(0xFFC00000 | (directoryIndex << 12));

			//*ptr = 0;

			//unsigned* x = (unsigned*)pt;
			//*x = 0;
			memset(pt, 0, sizeof(PageTable));
			//for(;;);

			//__asm("xchg %%bx, %%bx\n" : : : "bx");

			//unsigned* x = (unsigned*)(&ptLogic->entries[1023]);
			//__asm("xchg %%bx, %%bx\n" : : : "bx");
			//*x = (unsigned)ptPhys | PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;

			//ptLogic->entries[1023].address = ((unsigned)ptPhys) >> 12;
			//ptLogic->entries[1023].flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;
			//__asm("xchg %%bx, %%bx\n" : : : "bx");
		}

		pte->address = ((unsigned)physAddress >> 12);
		pte->flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;

		//ptLogic->entries[tableIndex].address = ((unsigned)physAddress >> 12);
		//ptLogic->entries[tableIndex].flags = PAGE_TABLE_FLAG_PRESENT | PAGE_TABLE_FLAG_READ_WRITE | PAGE_TABLE_FLAG_SUPERVISOR;


		PutString(" Mapped!!! ");
		// Refresh cr3
		//__asm(
		//"mov %%cr3, %%eax \n"
		//"mov %%eax, %%cr3 \n"
		//: : : "eax");*/
	}

	void* Alloc(unsigned allocSize)
	{
		return AllocPhys(allocSize);
	}

	void PrintMemoryMap()
	{
		PutString("===== Memory map =====\n");

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
			}

			currentMap = currentMap->next;
		}
	}
}
