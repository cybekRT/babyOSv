#include "bootloader_info.h"

struct MemoryInfo_t
{
	long long base;
	long long length;
	unsigned int type;
	unsigned int attr;
	//char padding[8];
} __attribute__((__packed__));

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

	char* vmem = (char*)0xb8000;
	vmem[p] = c;
	p+=2;
}

void PutString(char* s)
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
	char* data = (char*)0xb8000;

	MemoryInfo_t* mi = (MemoryInfo_t*)(bootloader_info_ptr->memoryEntries);
	unsigned miSize = *bootloader_info_ptr->memoryEntriesCount;

	while(true)
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
	}

	for(;;)
	{
		__asm("cli");
		__asm("hlt");
	}
}