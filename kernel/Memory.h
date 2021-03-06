#pragma once

enum PageDirectoryFlag
{
	PAGE_DIRECTORY_FLAG_PRESENT		= (1 << 0),
	PAGE_DIRECTORY_FLAG_READONLY		= (0 << 1),
	PAGE_DIRECTORY_FLAG_READ_WRITE		= (1 << 1),
	PAGE_DIRECTORY_FLAG_USER		= (0 << 2),
	PAGE_DIRECTORY_FLAG_SUPERVISOR		= (1 << 2),
	PAGE_DIRECTORY_FLAG_WRITE_THROUGH	= (1 << 3),
	PAGE_DIRECTORY_FLAG_CACHE_ENABLED	= (0 << 4),
	PAGE_DIRECTORY_FLAG_CACHE_DISABLED	= (1 << 4),
	PAGE_DIRECTORY_FLAG_ACCESSED		= (1 << 5),
	PAGE_DIRECTORY_FLAG_PAGE_4K		= (0 << 7),
	PAGE_DIRECTORY_FLAG_PAGE_4M		= (1 << 7),
};

enum PageTableFlag
{
	PAGE_TABLE_FLAG_PRESENT			= (1 << 0),
	PAGE_TABLE_FLAG_READONLY		= (0 << 1),
	PAGE_TABLE_FLAG_READ_WRITE		= (1 << 1),
	PAGE_TABLE_FLAG_USER			= (0 << 2),
	PAGE_TABLE_FLAG_SUPERVISOR		= (1 << 2),
	PAGE_TABLE_FLAG_WRITE_THROUGH		= (1 << 3),
	PAGE_TABLE_FLAG_CACHE_ENABLED		= (0 << 4),
	PAGE_TABLE_FLAG_CACHE_DISABLED		= (1 << 4),
	PAGE_TABLE_FLAG_ACCESSES		= (1 << 5),
	PAGE_TABLE_FLAG_DIRTY			= (1 << 6),
	PAGE_TABLE_FLAG_GLOBAL			= (1 << 7),
	PAGE_TABLE_ADDRESS_OFFSET		= 12,
};

struct PageDirectoryEntry
{
	unsigned flags : 12;
	unsigned address : 20;

	void* GetAddress() { return (void*)(address << 12); }
	//void SetAddress(void* address) { this->address = ((unsigned)address) >> 12; } // TODO: add assert

	bool IsUsed() { return flags & PAGE_DIRECTORY_FLAG_PRESENT; }
} __attribute__((packed));

struct PageDirectory
{
	PageDirectoryEntry entries[1024];
} __attribute__((packed));

typedef PageDirectoryEntry PageTableEntry;

typedef PageDirectory PageTable;

void memset(void* ptr, unsigned char c, unsigned len);
void memcpy(void* dst, void* src, u32 len);

namespace Memory
{
	bool Init();
	
	namespace Physical
	{
		struct MemoryMap
		{
			void* address[340];
			unsigned length[340];
			bool used[340];

			MemoryMap* next;
		};

		extern MemoryMap memoryMap;

		void InsertMemoryMapEntry(void* address, unsigned length);
		void SortMemoryMap();
		void MergeMemoryMap();
		void PrintMemoryMap();

		// Physical allocator
		void* Alloc(unsigned allocSize);
		void Free(void* address);
	}

	namespace Logical
	{
		void DisableFirstMegabyteMapping();

		// Mapper
		void* Map(void* physAddress, void* logicAddress, unsigned length);
		void Unmap(void* logicAddress);

		// Logical + physical allocator
		void* Alloc(unsigned allocSize);
		void Free(void* ptr);
	}

	void* Alloc(unsigned bytes);
	void Free(void* ptr);

	template<class T>
	T* Alloc()
	{
		T* v = (T*)Alloc(sizeof(T));
		//v->T();
		return v;
	}

	template<class T>
	void Free(T* ptr)
	{
		//ptr->~T();
		Memory::Free((void*)ptr);
	}
}
