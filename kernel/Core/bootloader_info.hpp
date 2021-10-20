struct bootloader_info_t
{
	unsigned int* memoryEntriesCount;
	void* memoryEntries;
	void* pageDirectory;
} __attribute((__packed__));
