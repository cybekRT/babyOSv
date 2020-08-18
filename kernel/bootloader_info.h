typedef struct
{
	unsigned int* memoryEntriesCount;
	void* memoryEntries;
	void* pageDirectory;
} __attribute((__packed__)) bootloader_info_t;

extern bootloader_info_t* bootloader_info_ptr;
