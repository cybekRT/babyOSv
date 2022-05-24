#include "Memory.hpp"
#include "bootloader_info.hpp"
#include "preinit.hpp"

// TODO: stop using bubble sort everywhere! Optimize sorting algorithms in allocator~!

struct gdt_entry_bits
{
	u32 limit_low              : 16;
	u32 base_low               : 24;
	u32 accessed               :  1;
	u32 read_write             :  1; // readable for code, writable for data
	u32 conforming_expand_down :  1; // conforming for code, expand down for data
	u32 code                   :  1; // 1 for code, 0 for data
	u32 code_data_segment      :  1; // should be 1 for everything but TSS and LDT
	u32 DPL                    :  2; // privilege level
	u32 present                :  1;
	u32 limit_high             :  4;
	u32 available              :  1; // only used in software; has no effect on hardware
	u32 long_mode              :  1;
	u32 big                    :  1; // 32-bit opcodes for code, uint32_t stack for data
	u32 gran                   :  1; // 1 to use 4k page addressing, 0 for byte addressing
	u32 base_high              :  8;
} __attribute__((packed));

struct gdt_entry
{
	u16 size;
	gdt_entry_bits* entries;
} __attribute__((packed));

struct tss_entry_struct {
	u32 prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
	u32 esp0;     // The stack pointer to load when changing to kernel mode.
	u32 ss0;      // The stack segment to load when changing to kernel mode.
	// Everything below here is unused.
	u32 esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
	u32 ss1;
	u32 esp2;
	u32 ss2;
	u32 cr3;
	u32 eip;
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;
	u32 edi;
	u32 es;
	u32 cs;
	u32 ss;
	u32 ds;
	u32 fs;
	u32 gs;
	u32 ldt;
	u16 trap;
	u16 iomap_base;
} __attribute__((packed));

tss_entry_struct globalTSS;

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

	MemoryInfo_t* memoryEntries = nullptr;
	unsigned memoryEntriesCount = 0;

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
		unsigned kEnd = _kernel_end & 0x7FFFFFFF;
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
		unsigned memoryMapCount = 0;
		for(unsigned a = 0; a < memoryEntriesCount; a++)
		{
			if(memoryEntries[a].type != 1 || memoryEntries[a].length == 0)
				continue;
			if(memoryEntries[a].base > 0xffffffff)
				continue;
			if(memoryEntries[a].base + memoryEntries[a].length > 0xffffffff)
				memoryEntries[a].length = 0xffffffff - memoryEntries[a].base;

			// Physical::memoryMap.address[memoryMapCount] = (void*)memoryEntries[a].base;
			// Physical::memoryMap.length[memoryMapCount] = memoryEntries[a].length;

			if(memoryEntries[a].base > 0xFFFFFFFF)
				continue;
			if(memoryEntries[a].length > 0xFFFFFFFF)
				memoryEntries[a].length = 0xFFFFFFFF;

			Memory::Physical::AddFreeMemory((void*)memoryEntries[a].base, (u32)memoryEntries[a].length);

			memoryMapCount++;
		}

		Memory::Physical::ReserveMemory((void*)0xA0000, 64 * 1024);
		Memory::Physical::ReserveMemory((void*)(_kernel_beg & (~0x80000000)), _kernel_end - _kernel_beg);
	}

	void CreateTSS()
	{
		static gdt_entry gdt;
		__asm("sgdt %0" : "=m"(gdt));

		Print("GDT: %x - %d\n", gdt.entries, gdt.size);

		u32 base = (u32) &globalTSS;
		u32 limit = sizeof(globalTSS);

		gdt_entry_bits* g = gdt.entries + 5;
		// Add a TSS descriptor to the GDT.
		g->limit_low = limit;
		g->base_low = base;
		g->accessed = 1; // With a system entry (`code_data_segment` = 0), 1 indicates TSS and 0 indicates LDT
		g->read_write = 0; // For a TSS, indicates busy (1) or not busy (0).
		g->conforming_expand_down = 0; // always 0 for TSS
		g->code = 1; // For a TSS, 1 indicates 32-bit (1) or 16-bit (0).
		g->code_data_segment=0; // indicates TSS/LDT (see also `accessed`)
		g->DPL = 0; // ring 0, see the comments below
		g->present = 1;
		g->limit_high = (limit & (0xf << 16)) >> 16; // isolate top nibble
		g->available = 0; // 0 for a TSS
		g->long_mode = 0;
		g->big = 0; // should leave zero according to manuals.
		g->gran = 0; // limit is in bytes, not pages
		g->base_high = (base & (0xff << 24)) >> 24; //isolate top byte

		// Ensure the TSS is initially zero'd.
		memset(&globalTSS, 0, sizeof(globalTSS));

		globalTSS.ss0  = 0x10;  // Set the kernel stack segment.
		globalTSS.esp0 = (u32)Memory::Alloc(8192);// REPLACE_KERNEL_STACK_ADDRESS; // Set the kernel stack pointer.
		globalTSS.esp0 += 8192;

		Print("Memory: %x:%p\n", globalTSS.ss0, globalTSS.esp0);

		// for(;;);

		__asm(
			"mov $(5*8 | 0), %%ax \r\n"
			"ltr %%ax \r\n"
			: : : "ax"
		);
	}

	bool Init()
	{
		PutString("Memory init...\n");

		Physical::Init();
		Logical::Init();

		memoryEntries = (MemoryInfo_t*)(((unsigned)_bootloader_info_ptr->memoryEntries) | 0x80000000);
		memoryEntriesCount = *_bootloader_info_ptr->memoryEntriesCount;

		Logical::DisableFirstMegabyteMapping();
		SortEntries();
		FixEntries();
		CreateMemoryMap();
		CreateTSS();

		return true;
	}

	template<typename T>
	void Swap(T &a, T &b)
	{
		T old = a;
		a = b;
		b = old;
	}

	struct MallocHeader
	{
		u32 length;
		MallocHeader* next;
		u8 used;
		u8 padding[7];
	} __attribute__((packed));

	MallocHeader* mallocPage = nullptr;

	void* MallocAddPage(unsigned bytes)
	{
		u32 totalSize = ((bytes + sizeof(MallocHeader)) + PAGE_SIZE - 1) & (~(PAGE_SIZE - 1));

		u32 minSize = 4 * PAGE_SIZE;
		if(totalSize < minSize)
			totalSize = minSize;

		auto newHeader = (MallocHeader*)Logical::Alloc(totalSize); // TODO: use pages count
		ASSERT(newHeader != nullptr, "Out of memory :(");
		if(!newHeader)
			return nullptr;

		newHeader->length = totalSize - sizeof(MallocHeader);
		newHeader->next = nullptr;
		newHeader->used = false;

		// if(mallocPage == nullptr)
		// {
		// 	mallocPage = newHeader;
		// }
		// else
		// {
		// 	auto ptr = mallocPage;
		// 	while(ptr->next != nullptr)
		// 	{
		// 		if(mallocPage < ptr->next)
		// 			break;
		// 		ptr = ptr->next;
		// 	}

		// 	if(ptr->next != nullptr)
		// 		newHeader->next = ptr->next;
		// 	ptr->next = newHeader;
		// }

		return newHeader;
	}

	void* Alloc(unsigned bytes)
	{
		// Print("Inside~! ");
		// bytes += sizeof(MallocHeader);

		if(mallocPage == nullptr)
		{
			Print("Resizing malloc 1\n");
			mallocPage = (MallocHeader*)MallocAddPage(bytes);
			ASSERT(mallocPage, "Out of memory");
			if(!mallocPage)
				return nullptr;
		}

		auto ptr = mallocPage;
		MallocHeader* prevPtr = nullptr;
		int xx = 0;
		//while(ptr)
		// Print("Req: %d => ", bytes);
		for(;;)
		{
			xx++;
			// Print("(%d) ", ptr->length);
			if(ptr->length >= bytes)
			{
				// auto ptr = mallocPage;
				if(ptr->length - bytes > sizeof(MallocHeader))
				{
					MallocHeader* nextHdr = (MallocHeader*)(((u8*)ptr) + bytes + sizeof(MallocHeader));
					nextHdr->length = ptr->length - bytes - sizeof(MallocHeader);
					nextHdr->next = ptr->next;
					nextHdr->used = false;

					ptr->next = nextHdr;
					ptr->length = bytes;
				}

				ptr->used = 1;
				// mallocPage = mallocPage->next;
				if(prevPtr)
					prevPtr->next = ptr->next;
				else
					mallocPage = ptr->next;

				// Print("\n");
				return ptr + 1;
			}

			prevPtr = ptr;
			ptr = ptr->next;

			if(ptr == nullptr)
			{
				Print("Resizing malloc 2\n");
				ptr = (MallocHeader*)MallocAddPage(bytes);
				ASSERT(ptr != nullptr, "Out of memory");

				prevPtr->next = ptr;
			}
		}

		Print("Oopsie... %d\n", xx);
		Physical::PrintMemoryMap();
		__asm("cli");
		__asm("hlt");
		FAIL("Oopsie...");

		return nullptr;
	}

	void MallocMerge()
	{
		auto ptr = mallocPage;
		u32 count = 0;
		while(ptr)
		{
			if((u8*)ptr->next == ((u8*)ptr) + ptr->length + sizeof(MallocHeader))
			{
				auto next = ptr->next;
				ptr->length += next->length + sizeof(MallocHeader);
				ptr->next = next->next;

				count++;
			}
			else
				ptr = ptr->next;
		}

		// Print("Merged: %d\n", count);
	}

	void Free(void* ptr)
	{
		// return;
		MallocHeader* hdr = (MallocHeader*)ptr;//&((MallocHeader*)ptr)[-1];
		hdr--;

		if(!hdr->used)
		{
			FAIL("Double free~!\n");
			return;
			//Print("Double free~!\n"); for(;;);
		}

		hdr->used = false;

		if(hdr < mallocPage)
		{
			hdr->next = mallocPage;
			mallocPage = hdr;
		}
		else
		{
			auto page = mallocPage;
			while(page)
			{
				if(page->next > hdr || page->next == nullptr)
				{
					hdr->next = page->next;
					page->next = hdr;
					break;
				}

				page = page->next;
			}
		}

		MallocMerge();

		//Print("MallocPage: %p, Hdr: %p\n", mallocPage, hdr);
		// if(hdr < mallocPage)
		// {
			// hdr->next = mallocPage;
			// mallocPage = hdr;
			//Print("MallocPage = Hdr = %p\n", mallocPage);
		// }
		// else
		// {
		// 	auto ptr = mallocPage;
		// 	auto prevPtr = (MallocHeader*)nullptr;
		// 	while(ptr)
		// 	{
		// 		if(ptr == hdr || ptr->next == hdr)
		// 		{
		// 			FAIL("Cycle in malloc list~!\n");
		// 			//Print("Oopsie...\n"); for(;;);
		// 		}

		// 		if(ptr->next > hdr)
		// 		{
		// 			hdr->next = ptr->next;
		// 			ptr->next = hdr;

		// 			break;
		// 		}

		// 		prevPtr = ptr;
		// 		ptr = ptr->next;
		// 	}

		// 	if(ptr == nullptr)
		// 	{
		// 		// Print("Prev: %p, Ptr: %p, Hdr: %p, Hdr->next: %p\n", prevPtr, ptr, hdr, hdr->next);
		// 		prevPtr->next = hdr;
		// 		//hdr->next = nullptr;
		// 	}
		// }
		// TODO: merge entries
	}

	u32 Size(void* ptr)
	{
		MallocHeader* hdr = &((MallocHeader*)ptr)[-1];
		return hdr->length - sizeof(MallocHeader);;
	}

	void PrintMemoryMap()
	{
		Physical::PrintMemoryMap();

		u32 totalLength = 0;

		unsigned pagesCount = 0;
		auto ptr = mallocPage;
		Print("Pages:");
		while(ptr)
		{
			Print("(%p, %d) ", ptr, ptr->length);
			pagesCount++;
			totalLength += ptr->length;
			ptr = ptr->next;
		}

		Print("Malloc pages: %d\n", pagesCount);
		Print("Total available bytes: %d\n", totalLength);
	}
}

void* operator new(size_t size)
{
	ASSERT(size > 0, "Invalid size");

	// Print("New: %d bytes - ", size);
	auto ptr = Memory::Alloc(size);
	// memset(ptr, 0, size);
	ASSERT(ptr != (void*)0x1100, "WTF"); // FIXME: fix me~!
	// Print("%p\n", ptr);
	return ptr;
}

void* operator new[](size_t size)
{
	ASSERT(size > 0, "Invalid size");

	// Print("New[]: %d bytes - ", size);
	auto ptr = Memory::Alloc(size);
	// memset(ptr, 0, size);
	ASSERT(ptr != (void*)0x1100, "WTF"); // FIXME: fix me~!
	// Print("%p\n", ptr);
	return ptr;
}

void operator delete(void* ptr, size_t size)
{
	ASSERT(size > 0, "Invalid size");

	// Print("Delete: %p - %d\n", ptr, size);
	Memory::Free(ptr);
}

void operator delete(void* ptr)
{
	// Print("Delete: %p - ...\n", ptr);
	Memory::Free(ptr);
}

void operator delete[](void* ptr, size_t size)
{
	ASSERT(size > 0, "Invalid size");

	// Print("Delete[]: %p - %d\n", ptr, size);
	Memory::Free(ptr);
}

void operator delete[](void* ptr)
{
	// Print("Delete[]: %p - ...\n", ptr);
	Memory::Free(ptr);
}
