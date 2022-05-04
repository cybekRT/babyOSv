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
#include"Mutex.hpp"
#include"Shell.hpp"
#include"Video/VGA.hpp"
#include"Video/Video.hpp"
#include"Video/Video_VGA.hpp"

Video::Bitmap* screen = nullptr;

void TestFuncBody()
{
	u8* p = (u8*)0x800a0000;
	u8 r = 0, g = 0, b = 0;
	for(;;)
	{
		for(unsigned a = 0; a < 320*200; a++)
			//p[a] = c;
			Video::SetPixel(a % 320, a / 320, Video::Color(r, g, b));

		//c++;
		r+=32;
		if(r == 0)
		{
			g+=32;
			if(g == b)
				b+=32;
		}

		Video::DrawRect(screen, Video::Rect(10, 10, 180, 50), Video::Color(255, 0, 0));
		Video::DrawRect(screen, Video::Rect(40, 30, 180, 20), Video::Color(0, 255, 0, 128));
		Video::UpdateScreen();

		for(volatile unsigned a = 0; a < 100000; a++)
		{
			__asm("int $255");
		}
	}
}

void TestFunc()
{
	__asm(
		"mov $(4*8 | 3), %%ax \r\n"
		"mov %%ax, %%ds \r\n"
		"mov %%ax, %%es \r\n"
		"mov %%ax, %%fs \r\n"
		"mov %%ax, %%gs \r\n"
		"mov %%esp, %%eax \r\n"
		"push $(4*8 | 3) \r\n"
		"push %%eax \r\n"
		"pushf \r\n"
		"push $(3*8 | 3) \r\n"
		"push $%0 \r\n"
		"xchg %%bx, %%bx \r\n"
		"iret \r\n"
		: : "m"(TestFuncBody)
	);
}

Mutex m;
int YoLo(void*)
{
	for(;;)
	{
		m.Lock();
		Interrupt::Disable();
		Thread::SetState(nullptr, Thread::State::Unstoppable);

		u32 cx, cy;
		u8 cc[2];
		Terminal::GetColor(cc, cc+1);
		Terminal::GetXY(&cx, &cy);

		Terminal::SetColor(0xE, 0x3);
		Terminal::SetXY(1, 0);

		for(unsigned a = 0; a < 80; a++)
		{
			((u8*)0x800b8000)[a*2+0] = 0x00;
			((u8*)0x800b8000)[a*2+1] = 0x3E;
		}

		Terminal::Print("Uptime: %d", (u32)Timer::GetTicks()/1000);

		Terminal::SetColor(cc[0], cc[1]);
		Terminal::SetXY(cx, cy);

		Interrupt::Enable();
		Timer::Delay(500);
		m.Unlock();
		// Thread::NextThread();
	}

	return 0;
}

int YoLo2(void*)
{
	for(;;)
	{
		m.Lock();
		Interrupt::Disable();

		((u8*)0x800b8000)[79*2+0]++;// = 0x00;

		Interrupt::Enable();
		Timer::Delay(100);
		m.Unlock();
	}
}

int FakeKbdThread(void*)
{
	Timer::Delay(1000);

	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = 's' } ); Timer::Delay(200);
	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = 'p' } ); Timer::Delay(200);
	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = 'l' } ); Timer::Delay(200);
	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = 'a' } ); Timer::Delay(200);
	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = 's' } ); Timer::Delay(200);
	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .ascii = 'h' } ); Timer::Delay(200);
	Keyboard::AddEvent(Keyboard::KeyEvent { .type = Keyboard::KeyType::Pressed, .key = Keyboard::KeyCode::Enter } );

	return 0;
}

extern "C" void __cxa_pure_virtual()
{
	ASSERT(false, "Pure virtual function calles :(");
}

extern u32* _ctors_beg;
extern u32* _ctors_end;

namespace Floppy
{
	u8 Read(void* dev, u32 lba, u8* buffer);
	u8 Write(void* dev, u32 lba, u8* buffer);
}

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

#if 0
	VGA::Init();

	u8* vgaPtr = (u8*)0x800a0000;
	for(unsigned a = 0; a < 320*200; a++)
		vgaPtr[a] = a % 320;

	__asm("cli");
	for(;;)
		HALT;
#endif

	Thread::Thread* testThread;
	Thread::Create(&testThread, (u8*)"YoLo", YoLo);

	Block::Init();
	Block::Dummy::Init();
	ISA_DMA::Init();
	Floppy::Init();
	ATA::Init();

#if 0
	u8 tmpBuf[512];
	Print("Reading...\n");
	Floppy::Read(nullptr, 0, tmpBuf);
	tmpBuf[510] = 0x00;
	tmpBuf[511] = 0x00;

	for(unsigned a = 0; a < 512; a++)
		tmpBuf[a] = 0x00;

	Timer::Delay(3000);

	Print("Writing...\n");
	for(unsigned a = 0; a < 10; a++)
	{
		if(!Floppy::Write(nullptr, 0, tmpBuf))
			break;
	}

	for(;;) HALT;
#endif

	FS::Init();
	FAT::Init();
	FS::BlkFS::Init();

#if 1
	VFS::Init();

	if(VFS::Mount("fdd0r1", "fdd") != Status::Success)
	{
		Print("No floppy :<\n");
		for(;;);
	}

	VFS::Mount("hdd0p3", "hdd");

	Thread::Start(testThread);

	Thread::Thread* testThread2;
	Thread::Create(&testThread2, (u8*)"YoLo2", YoLo2);
	Thread::Start(testThread2);

#if 0
	{
		FS::Directory* dir;
		VFS::DirectoryOpenRoot(&dir);

		auto bd = &Block::devices[0];
		auto fs = FS::filesystems.Front();
		void* fsPriv;

		fs->Alloc(bd, &fsPriv);
		fs->OpenRoot(fsPriv, &dir);

		Print("Searching splash\n");
		FS::DirEntry entry;
		while(fs->ReadDirectory(fsPriv, dir, &entry) == FS::Status::Success)
		{
			if(!strcmp((char*)entry.name, "splash"))
			{
				Print("Found~!\n");
				FS::File* f;
				fs->OpenFile(fsPriv, dir, &f);

				u8* dst = (u8*)(0x80000000 | 0xA0000);
				u32 r;
				Print("Reading!!!!\n");
				fs->ReadFile(fsPriv, f, dst, 64000, &r);
				Print("Read %u bytes!\n", r);

				break;
			}
		}
		for(;;);
	}
#endif

	FS::File* logoFile;
	VFS::FileOpen("/fdd0/images/logo.bmp", &logoFile);
	Print("<><><><><><>\n");

if(0)
{
	Video::SetDriver(&Video::vgaDriver);

	Container::LinkedList<Video::Mode> modes;
	Video::GetAvailableModes(modes);

	Print("Available modes:\n");
	for(auto mode : modes)
	{
		Print("- [%c] %dx%d %dbpp\n", (mode.type == Video::Mode::Type::Graphical) ? 'G' : 'T', mode.width, mode.height, mode.bpp);
	}

	Video::SetMode(modes.Back());
	Video::Clear();

	screen = Video::GetScreen();

	TestFunc();

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
#endif

	Print("\n\nKernel running...\n");
	for(;;)
	{
		Thread::SetState(Thread::currentThread, Thread::State::Zombie);
		Thread::NextThread();
		Print("WTF~!\n");
	}
}