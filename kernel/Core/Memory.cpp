#include "Memory.h"
#include "bootloader_info.h"

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
		memset((void*)&Physical::memoryMap, 0, sizeof(Physical::memoryMap));

		unsigned memoryMapCount = 0;
		for(unsigned a = 0; a < memoryEntriesCount; a++)
		{
			if(memoryEntries[a].type != 1 || memoryEntries[a].length == 0)
				continue;
			if(memoryEntries[a].base > 0xffffffff)
				continue;
			if(memoryEntries[a].base + memoryEntries[a].length > 0xffffffff)
				memoryEntries[a].length = 0xffffffff - memoryEntries[a].base;

			Physical::memoryMap.address[memoryMapCount] = (void*)memoryEntries[a].base;
			Physical::memoryMap.length[memoryMapCount] = memoryEntries[a].length;

			memoryMapCount++;
		}
	}

	bool Init()
	{
		PutString("Memory init...\n");

		memoryEntries = (MemoryInfo_t*)(((unsigned)_bootloader_info_ptr->memoryEntries) | 0x80000000);
		memoryEntriesCount = *_bootloader_info_ptr->memoryEntriesCount;

		Logical::DisableFirstMegabyteMapping();
		SortEntries();
		FixEntries();
		CreateMemoryMap();

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

	bool MallocResize(unsigned bytes)
	{
		u32 totalSize = ((bytes + sizeof(MallocHeader)) + 0xFFF) & (~0xFFF);

		u32 minSize = 4 * 4096;
		if(totalSize < minSize)
			totalSize = minSize;

		auto newHeader = (MallocHeader*)Logical::Alloc(totalSize); // TODO: use pages count
		if(!newHeader)
			return false;

		newHeader->length = totalSize - sizeof(MallocHeader);
		newHeader->next = nullptr;
		newHeader->used = false;

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

	void* Alloc(unsigned bytes)
	{
		bytes += sizeof(MallocHeader);

		if(mallocPage == nullptr)
		{
			if(!MallocResize(bytes))
				return nullptr;
		}

		// First fit
		auto ptr = mallocPage;
		auto prevPtr = (MallocHeader*)nullptr;
		while(ptr)
		{
			if(ptr->length >= bytes)
			{
				if(ptr->length > bytes + sizeof(MallocHeader))
				{
					auto oldLength = ptr->length;
					auto oldNext = ptr->next;
					auto newHeader = (MallocHeader*)(((char*)ptr) + bytes);
					ptr->length = bytes;
					ptr->next = newHeader;
					ptr->used = true;

					newHeader->length = oldLength - bytes;
					newHeader->next = oldNext;
					newHeader->used = false;
				}

				if(prevPtr)
					prevPtr->next = ptr->next;
				else
				{
					mallocPage = ptr->next;
					if(!mallocPage)
					{
						Memory::Physical::PrintMemoryMap();
						for(;;)
						{
							__asm("hlt");
						}
					}
					ASSERT(mallocPage, "Out of memory?");
					//Print("MallocPage = ptr->next = %p", ptr->next);
				}

				ptr->next = nullptr;

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

	void Free(void* ptr)
	{
		MallocHeader* hdr = &((MallocHeader*)ptr)[-1];

		if(!hdr->used)
		{
			return;
			FAIL("Double free~!\n");
			//Print("Double free~!\n"); for(;;);
		}

		hdr->used = false;

		//Print("MallocPage: %p, Hdr: %p\n", mallocPage, hdr);
		if(hdr < mallocPage)
		{
			hdr->next = mallocPage;
			mallocPage = hdr;
			//Print("MallocPage = Hdr = %p\n", mallocPage);
		}
		else
		{
			auto ptr = mallocPage;
			auto prevPtr = (MallocHeader*)nullptr;
			while(ptr)
			{
				if(ptr == hdr || ptr->next == hdr)
				{
					FAIL("Cycle in malloc list~!\n");
					//Print("Oopsie...\n"); for(;;);
				}

				if(ptr->next > hdr)
				{
					hdr->next = ptr->next;
					ptr->next = hdr;

					break;
				}

				prevPtr = ptr;
				ptr = ptr->next;
			}

			if(ptr == nullptr)
			{
				Print("Prev: %p, Ptr: %p, Hdr: %p, Hdr->next: %p\n", prevPtr, ptr, hdr, hdr->next);
				prevPtr->next = hdr;
				//hdr->next = nullptr;
			}
		}
		// TODO: merge entries
	}

	u32 Size(void* ptr)
	{
		MallocHeader* hdr = &((MallocHeader*)ptr)[-1];
		return hdr->length - sizeof(MallocHeader);;
	}
}

//#include<cstddef>

//extern "C"
//{
	void* operator new(size_t size)
	{
		//Print("New: %d bytes\n", size);
		return Memory::Alloc(size);
	}

	void* operator new[](size_t size)
	{
		//Print("New[]: %d bytes\n", size);
		return Memory::Alloc(size);
	}

	void operator delete(void* ptr, size_t size)
	{
		//Print("Delete: %p - %d\n", ptr, size);
		Memory::Free(ptr);
	}

	void operator delete(void* ptr)
	{
		//Print("Delete: %p - ...\n", ptr);
	}

	void operator delete[](void* ptr, size_t size)
	{
		//Print("Delete[]: %p - %d\n", ptr, size);
		Memory::Free(ptr);
	}

	void operator delete[](void* ptr)
	{
		//Print("Delete[]: %p - ...\n", ptr);
	}
//}
