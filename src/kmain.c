#include "bootloader_info.h"

extern bootloader_info_t* bootloader_info_ptr;
//extern unsigned* kernel_end;

extern unsigned* kernelx;
unsigned* kernel_end;

char* tmp = "Dafuq, just random text~!";

typedef struct
{
	long long base;
	long long length;
	unsigned int type;
	unsigned int attr;
	char padding[8];
} __attribute((__packed)) MemoryInfo_t;

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

void kmain()
{
	kernel_end = kernelx;
	/*char* vid = (char*)0xa0000;

	for(unsigned a = 0; a < 320 * 3; a++)
	{
		vid[a] = 0x1;
		vid[320 * 197 + a] = 0x1;
	}

	for(unsigned a = 0; a < 200; a++)
	{
		vid[a * 320 + 0] = 0x1;
		vid[a * 320 + 1] = 0x1;
		vid[a * 320 + 2] = 0x1;

		vid[a * 320 + 0 + 317] = 0x1;
		vid[a * 320 + 1 + 317] = 0x1;
		vid[a * 320 + 2 + 317] = 0x1;
	}*/

	char* data = (char*)0xb8000;
	for(unsigned a = 0; a < 80*25; a++)
	{
		data[a * 2] = 0;
	}

	/*for(unsigned a = 0; a < 320*200; a++)
	{
		data[a * 2 + 0] = 0;
		//data[a * 2 + 1] = 0x80;
	}*/

//__asm("xchgb bx, bx");
	data[ 0] = 'K';
	data[ 2] = 'e';
	data[ 4] = 'r';
	data[ 6] = 'n';
	data[ 8] = 'C';
	data[10] = ' ';

	for(unsigned a = 0; a < strlen(tmp); a++)
	{
		data[80 * 2 + a * 2] = tmp[a];
	}

	char map[] = "0123456789ABCDEF";

	int v = HAL_In(0x21);
	/*data[0] = '0';
	data[2] = 'x';
	data[4] = map[v >> 4];
	data[6] = map[v & 0xf];
	data[8] = ' ';*/

	/*PutChar('0');
	PutChar('x');
	PutChar(map[v >> 4]);
	PutChar(map[v & 0xf]);
	PutChar(' ');*/
	//PutHex(0xBAADF00D);

	//for(;;);

	PutChar('_');
	PutHex(bootloader_info_ptr);
	PutChar('_\n');

	//PutString("Yolo\nYolo2\nYolo3!!");
	//PutHex(0xBAADF00Du);

	MemoryInfo_t* mi = (MemoryInfo_t*)(bootloader_info_ptr->memoryEntries);
	PutHex(mi); PutChar('\n');
	unsigned miSize = *bootloader_info_ptr->memoryEntriesCount;//*(((unsigned*)bootloader_info_ptr)-1);
	//unsigned miSize = *(((unsigned*)bootloader_info_ptr)+2*24);

	PutHex(miSize);
	//for(;;);

	for(unsigned a = 0; a < miSize; a++)
	{
		//PutChar('.');
		//continue;

		PutString("--------\n");

		if(mi[a].base < kernel_end)
		{
			mi[a].base = kernel_end;
		}

		PutHex(mi[a].base); PutChar('\n');
		PutHex(mi[a].length); PutChar('\n');
	}

	for(;;)
	{
		__asm("cli");
		__asm("hlt");
	}
}