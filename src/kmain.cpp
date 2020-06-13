#include "bootloader_info.h"
#include "Memory.h"

int strlen(const char* str)
{
	unsigned len = 0;
	while(*str++)
		len++;

	return len;
}

unsigned char HAL_In(unsigned short port)
{
	unsigned char value;

	__asm(
	"mov %1, %%dx \r\n"
	"in %%dx, %%al \r\n"
	"mov %%al, %0 \r\n"
	: "=r"(value) 
	: "r"(port)
	: "eax", "edx");

	return value;
}

unsigned char HAL_Out(unsigned short port, unsigned char data)
{
	unsigned char value;

	__asm(
	"mov %0, %%dx \r\n"
	"mov %1, %%al \r\n"
	"out %%al, %%dx \r\n"
	: 
	: "r"(port), "r"(data)
	: "eax", "edx");

	return value;
}

void PutChar(char c)
{
	static int p = 0;

	if(c == '\n')
	{
		p += 160;
		p -= p % 160;
		return;
	}

	if(p >= 80*25*2)
		return;

	char* vmem = (char*)0x800b8000;
	vmem[p] = c;
	p+=2;
}

void PutString(const char* s)
{
	while(*s)
	{
		PutChar(*s);
		s++;
	}
}

void PutHex(unsigned long v)
{
	char map[] = "0123456789ABCDEF";
	for(unsigned a = 0; a < 8; a++)
	{
		int nibble = v >> 28;
		PutChar(map[nibble]);
		v <<= 4;
	}
}

extern "C" void kmain()
{
	unsigned short* data = (unsigned short*)0x800b8000;
	for(unsigned a = 0; a < 80 * 25 * 2; a++) data[a] = 0x0700;

	//MemoryInfo_t* mi = (MemoryInfo_t*)(((unsigned)bootloader_info_ptr->memoryEntries) | 0x80000000);
	//unsigned miSize = *bootloader_info_ptr->memoryEntriesCount;

	PutHex((unsigned)bootloader_info_ptr->pageDirectory); PutString("\n");

	Memory::Init(bootloader_info_ptr->pageDirectory, bootloader_info_ptr->memoryEntries, *bootloader_info_ptr->memoryEntriesCount);
	PutString("$$$ "); PutHex( *(unsigned*)0x80001000 ); PutString(" $$$\n");

	void* a = Memory::Alloc(512);
	void* b = Memory::Alloc(128);

	Memory::FreePhys(a);

	void* c = Memory::Alloc(1024);
	Memory::PrintMemoryMap();

	Memory::MapPhys(c, (void*)0x123000);

	__asm("xchg %%bx, %%bx\n" : : : "bx");

	//for(;;);

	unsigned* z = (unsigned*)0x123000;//  (unsigned*)z;
	for(unsigned a = 0; a < 512; a++)
	{
		z[a] = 0xBAADF00D;
	}

	/*while(true)
	{
		bool changed = false;

		for(unsigned a = 1; a < miSize; a++)
		{
			if(mi[a].base <= mi[a - 1].base)
			{
				MemoryInfo_t tmp = mi[a];
				mi[a] = mi[a - 1];
				mi[a - 1] = tmp;
				changed = true;
			}
		}

		if(!changed)
			break;
	}

	for(unsigned a = 0; a < miSize; a++)
	{
		PutString("--------\n");

		if(mi[a].base < (unsigned)kernel_end)
		{
			unsigned diff = (unsigned)kernel_end - mi[a].base;
			mi[a].base = (unsigned)kernel_end;
			mi[a].length -= diff;
		}

		PutHex(mi[a].base); PutChar('\n');
		PutHex(mi[a].length); PutChar('\n');
		PutHex(mi[a].type); PutChar('\n');
	}*/

	for(;;)
	{
		__asm("cli");
		__asm("hlt");
	}
}