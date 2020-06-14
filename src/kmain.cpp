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
	{
		unsigned short* src = (unsigned short*)0x800b80a0;
		unsigned short* dst = (unsigned short*)0x800b8000;

		for(unsigned a = 0; a < 80*24; a++)
		{
			*dst++ = *src++;
		}

		for(unsigned a = 0; a < 80; a++)
		{
			*dst++ = 0x0700;
		}

		p -= 160;
	}

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

	Memory::Init(bootloader_info_ptr->memoryEntries, *bootloader_info_ptr->memoryEntriesCount);

	void* c = Memory::Alloc(1024);

	PutString("Filling memory!\n");
	unsigned* z = (unsigned*)c;
	for(unsigned a = 0; a < 256; a++)
		z[a] = 0xbaadf00d;

	Memory::PrintMemoryMap();
	PutString("Kernel halted~!");
	for(;;)
	{
		__asm("cli");
		__asm("hlt");
	}
}