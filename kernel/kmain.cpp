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
	Interrupt::Enable();
	PutString("Kernel halted~!\n");

	//u8* x = (u8*)(0x1234);
	//*x = 5;

	Keyboard::KeyEvent keyEvent;
	for(;;)
	{
		/*while(Keyboard::HasData())
		{
			auto x = Keyboard::ReadData();
			PutString("Kbd: "); PutHex(x); PutString("\n");

			if(x == 0x32)
				Memory::PrintMemoryMap();
		}*/

		while(Keyboard::ReadEvent(&keyEvent))
		{
			Print("Type: %x, Mod: %x, Key: %s\n", keyEvent.type, keyEvent.mod, Keyboard::KeyCode2Str(keyEvent.key));
		}

		__asm("hlt");
	}
}