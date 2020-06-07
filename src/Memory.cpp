#include "Memory.h"

void PutString(const char* s);
void PutHex(unsigned long v);

namespace Memory
{


	bool Init(void* pageDirectory)
	{
		PutString("Memory init...\n");

		pageDirectory = (void*)(unsigned(pageDirectory) | 0x80000000);

__asm("xchg %%bx, %%bx" : : : "bx");
		// LOL, gcc generates NULL dereference and UD2 just after it
		unsigned* test = (unsigned*)1;
		*test = 0xbaadf00d;

		// Disable first megabyte
		PageDirectory* pd = (PageDirectory*)pageDirectory;
		pd->entries[0].flags = 0;
		pd->entries[0].address = 0;

		// Refresh cr3
		__asm(
		"mov %%cr3, %%eax \n"
		"mov %%eax, %%cr3 \n"
		: : : "eax");

		PutString("##### ");

		PageTable* pt = (PageTable*)(((unsigned)pd->entries[0].address) << 12);
		pt = (PageTable*)((unsigned)pt | 0x80000000);
		PutHex((unsigned)pt);
		PutString(" #####");

		return true;
	}

	void Quit()
	{
		// LOL, never happens...
	}
}
