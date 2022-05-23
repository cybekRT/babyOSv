#include"Terminal.hpp"
#include"Memory.hpp"
#include"Interrupt.hpp"
#include"Timer.hpp"
#include"Input/Keyboard.hpp"
#include"Thread.hpp"

#include"ISA_DMA.hpp"

#include"Block/Block.hpp"
#include"Block/Dummy.hpp"
#include"Block/Floppy.hpp"
#include"Block/ATA.hpp"

#include"FS/FS.hpp"
#include"FS/BlkFS.hpp"
#include"FS/FAT.hpp"
#include"FS/VFS.hpp"
#include"Shell.hpp"
#include"Video/VGA.hpp"
#include"Video/Video.hpp"
#include"Video/Video_VGA.hpp"

Video::Bitmap* screen = nullptr;

extern u32* _ctors_beg;
extern u32* _ctors_end;

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");
	ASSERT(sizeof(size_t) == 4, "size_t");

	Terminal::Init();
	VGA::SetCursor(false);
	Memory::Init();
	Interrupt::Init();

	// Print("Logical: %p\n", (void*)kmain);
	// Print("Physical: %p\n", Memory::Logical::GetPhysicalFromLogical((void*)kmain));
	// for(;;);

	/* Call global constructors */
	Print("Constructors: (%p - %p)\n", _ctors_beg, _ctors_end);
	for(u32* ptr = _ctors_beg; ptr != _ctors_end; ptr++)
	{
		void (*func)() = (void (*)())(*ptr);
		Print("- %p\n", func);
		ASSERT(func, "Invalid constructor function~!");
		if(func)
			func();
	}

	Timer::Init();
	Thread::Init();
	Keyboard::Init();

	if(0)
	{
		Print("=== Memory test ===\n");
		for(unsigned a = 0; a < 80*25; a++)
			Terminal::Print(" ");
		Terminal::SetXY(0, 0);
		Print("Memory test:\n");
		char c = 0;
		u32 zxc = 0;

		for(volatile unsigned a = 0; a < 1000; a++)
		{
			zxc++;
			if(a % 50 == 0)
			{
				// Terminal::SetXY(0, 1);
				Terminal::Clear();
				Terminal::SetXY(0, 0);
				Print("### Iteration: %d\n", zxc);
				Memory::PrintMemoryMap();
				// Timer::Delay(1000);
			}

			// Memory::Alloc(5);
			// if(a == 999)
			{
				const u32 cnt = 1000;
				static u8* ptr[cnt];

				for(unsigned x = 0; x < cnt; x++)
				{
					u32 size = (zxc % 123) + 5;
					// Print("&ptr[x] = %p\nAlloc~! ", &ptr[x]);
					ptr[x] = (u8*)Memory::Alloc(size);// Memory::Physical::AllocPage();
					// Print("Assign~! (%p, %p)\n", ptr[x], Memory::Logical::GetPhysicalFromLogical(ptr[x]));
					// ptr[x] = tmpPtr;
					// Print("Filling~! [%p, %p]\n", &ptr[x][0], &ptr[x][1023]);
					for(unsigned y = 0; y < size; y++)
					{
						const u32 v = 0xBAADF00D;
						u8* vv = (u8*)&v;
						ptr[x][y] = vv[y % 4];
					}
					// Print("Ok, -> %d (%d - %p)\n", zxc, x, ptr[x]);
				}

				// FIXME: memory is leaking...
				for(unsigned x = 0; x < cnt; x++)
					Memory::Free(ptr[x]); // Memory::Physical::FreePage(ptr[ (x * 3) % cnt ]);

				if(a == 999)
					a = 0;
			}

			// for(volatile unsigned b = 0; b < 9999; b++)
			{

			}

			// Print("%c\b", c++);
		}
	}

	Block::Init();
	// Block::Dummy::Init();
	ISA_DMA::Init();
	Floppy::Init();
	// ATA::Init();
	FS::Init();
	FAT::Init();
	// FS::BlkFS::Init();
	VFS::Init();
	// FIXME: memory allocator is crashing... :F

	if(VFS::Mount("fdd0r0", "fdd") != Status::Success)
	{
		Print("No floppy :<\n");
		// for(;;);
	}

	if(0)
	{
		Video::SetDriver(&Video::vgaDriver);

		List<Video::Mode> modes;
		Video::GetAvailableModes(modes);

		Print("Available modes:\n");
		for(auto mode : modes)
		{
			Print("- [%c] %dx%d %dbpp\n", (mode.type == Video::Mode::Type::Graphical) ? 'G' : 'T', mode.width, mode.height, mode.bpp);
		}

		Video::SetMode(modes.Back());
		Video::Clear();

		screen = Video::GetScreen();

		// Video::DrawRect(Video::Rect(10, 10, 180, 50), Video::Color(255, 0, 0));
		// Video::DrawRect(Video::Rect(40, 30, 180, 20), Video::Color(0, 255, 0, 128));
	}

#if 1
	// Thread::Thread* fakeKbdThread;
	// Thread::Create(&fakeKbdThread, (u8*)"FakeKbd", FakeKbdThread);
	// Thread::Start(fakeKbdThread);

	Thread::Thread* shellThread;
	Thread::Create(&shellThread, (u8*)"Shell", Shell::Thread);
	Thread::Start(shellThread);
#endif

	Print("\n\nKernel running...\n");
	for(;;)
	{
		Thread::SetState(Thread::currentThread, Thread::State::Zombie);
		Thread::NextThread();
		Print("WTF~!\n");
	}
}