#include"bootloader_info.h"
#include"Terminal.h"
#include"Memory.h"
#include"Interrupt.h"
#include"Timer.h"
#include"Keyboard.h"

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

void HAL_Out(unsigned short port, unsigned char data)
{
	__asm(
	"mov %0, %%dx \r\n"
	"mov %1, %%al \r\n"
	"out %%al, %%dx \r\n"
	: 
	: "r"(port), "r"(data)
	: "eax", "edx");
}

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");

	Terminal::Init();

	Memory::Init(bootloader_info_ptr->memoryEntries, *bootloader_info_ptr->memoryEntriesCount);

//	u32 stackLength = 8192;
//	u8* stack = (u8*)Memory::Alloc(stackLength);
//	__asm("mov %0, %%esp" : : "r"(stack + stackLength));

	Interrupt::Init();
	Timer::Init();
	Keyboard::Init();

	Print("Test: %x, %d, %u\nAnd newline x: %s", 0xbaadf00d, -67, 631, "Line1\nLine2\nLine3!!!\n");
	
	Memory::PrintMemoryMap();
	PutString("Kernel halted~!\n");

	for(;;)
	{
		while(Keyboard::HasData())
		{
			auto x = Keyboard::ReadData();
			PutString("Kbd: "); PutHex(x); PutString("\n");

			if(x == 0x32)
				Memory::PrintMemoryMap();
		}

		__asm("hlt");
	}
}