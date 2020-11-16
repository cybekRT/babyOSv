#include"Terminal.h"
#include"Memory.h"
#include"Interrupt.h"
#include"Timer.h"
#include"Keyboard.h"

#include"ISA_DMA.hpp"

#include"Block.hpp"
#include"Floppy.hpp"

#include"FS.hpp"
#include"FS_FAT12.hpp"

int strlen(const char* str)
{
	unsigned len = 0;
	while(*str++)
		len++;

	return len;
}

bool strcmp(char* a, char* b)
{
	while(*a || *b)
	{
		if(*a != *b)
			return false;

		a++;
		b++;
	}

	return true;
}

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");

	Terminal::Init();
	Memory::Init();

	Interrupt::Init();
	Timer::Init();
	Keyboard::Init();

	Print("Test: %x, %d, %u\nAnd newline x: %s", 0xbaadf00d, -67, 631, "Line1\nLine2\nLine3!!!\n");
	Memory::PrintMemoryMap();
	Interrupt::Enable();
	PutString("Kernel halted~!\n");

	ISA_DMA::Init();

	Block::Init();
	Floppy::Init();

	FS::Init();
	FS_FAT12::Init();

	Print("Mounting floppy...\n");

	auto dev = Block::devices.Front();
	auto fs = FS::filesystems.Front();
	void* fsPriv;

	fs->Alloc(dev, &fsPriv);

	char tmp[64];
	u8 tmpX = 0;
	Keyboard::KeyEvent keyEvent;
	Print("> ");
	for(;;)
	{
		while(Keyboard::ReadEvent(&keyEvent))
		{
			if(keyEvent.type == Keyboard::KeyType::Released)
				continue;

			if(keyEvent.key == Keyboard::KeyCode::Backspace)
			{
				if(tmpX > 0)
				{
					tmpX--;
					tmp[tmpX] = 0;
					Print("\b");
				}
			}
			else if(keyEvent.key == Keyboard::KeyCode::Enter)
			{
				tmp[tmpX] = 0;

				if(tmpX > 0)
				{
					Print("\nExecuting: %s", tmp);

					if(strcmp(tmp, "mem"))
					{
						Memory::PrintMemoryMap();
					}
					else if(strcmp(tmp, "fail"))
					{
						u8* x = (u8*)0x1234;
						*x = 5;
					}
				}
				Print("\n> ");

				tmpX = 0;
			}
			else if(keyEvent.ascii)
			{
				tmp[tmpX] = keyEvent.ascii;
				tmpX++;
				Print("%c", keyEvent.ascii);
			}
		}

		__asm("hlt");
	}
}