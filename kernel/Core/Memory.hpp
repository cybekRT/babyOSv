#pragma once

enum class PageSize : u32
{
	Page4kB,
	Page4MB,
};

enum class CacheType : u32
{
	Disabled = 0b01,
	Reserved = 0b11,
	WriteBack = 0b00,
	WriteThrough = 0b10,
};

enum class MemoryMode
{
	ReadOnly = 0,
	ReadWrite = 1,
};

enum class PrivilegeMode
{
	Supervisor = 0,
	UserAccess = 1,
};

// struct PageDirectoryFlags
// {
// 	u32 present					: 1;
// 	MemoryMode memoryMode		: 1;
// 	PrivilegeMode privilegeMode	: 1;
// 	// u32 cacheWriteThrough	: 1;
// 	// u32 cacheDisabled		: 1;
// 	CacheType cacheType			: 2;
// 	u32 accessed				: 1;
// 	u32 available0				: 1;
// 	PageSize pageSize			: 1;
// 	u32 available1_4			: 4;
// 	// Address
// } __attribute__((packed));

struct PageTableFlags
{

} __attribute__((packed));

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
	u32 present					: 1;
	MemoryMode memoryMode		: 1;
	PrivilegeMode privilegeMode	: 1;
	CacheType cacheType			: 2;
	u32 accessed				: 1;
	u32 available0				: 1;
	PageSize pageSize			: 1;
	u32 available1_4			: 4;
	unsigned address			: 20;

	void* GetAddress() { return (void*)(address << 12); }
	void SetAddress(void* address) { this->address = ((unsigned)address) >> 12; }

	bool IsUsed() { return present; }
} __attribute__((packed));

struct PageTableEntry
{
	u32 present					: 1;
	MemoryMode memoryMode		: 1;
	PrivilegeMode privilegeMode	: 1;
	CacheType cacheType			: 2;
	u32 accessed				: 1;
	u32 dirty					: 1;
	u32 pageAttributeTable		: 1;
	u32 global					: 1;
	u32 available0_2			: 3;
	unsigned address			: 20;

	void* GetAddress() { return (void*)(address << 12); }
	void SetAddress(void* address) { this->address = ((unsigned)address) >> 12; }

	bool IsUsed() { return present; }
} __attribute__((packed));

struct PageDirectory
{
	PageDirectoryEntry entries[1024];
} __attribute__((packed));

struct PageTable
{
	PageTableEntry entries[1024];
} __attribute__((packed));

// typedef PageDirectoryEntry PageTableEntry;

// typedef PageDirectory PageTable;

namespace Memory
{
	constexpr u32 PAGE_SIZE = 4096;

	bool Init();

	namespace Physical
	{
		bool Init();

		void AddFreeMemory(void* address, u32 length);
		void ReserveMemory(void* address, u32 length);

		// void InsertMemoryMapEntry(void* address, unsigned length);
		// void SortMemoryMap();
		// void MergeMemoryMap();
		void PrintMemoryMap();

		// Physical allocator
		void* Alloc(unsigned allocSize);
		void Free(void* address);

		void* AllocPage();
		void FreePage(void* addr);
	}

	namespace Logical
	{
		bool Init();
		void* GetPhysicalFromLogical(void* ptr);

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
	u32 Size(void* ptr);

	void PrintMemoryMap();

	template<class T>
	T* Alloc()
	{
		T* v = (T*)Alloc(sizeof(T));
		//new(v) T();
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
