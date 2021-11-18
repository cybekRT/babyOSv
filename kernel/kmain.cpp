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

bool shellReady = false;
u8 tolower(u8 c);

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
		//Print(".");

		Timer::Delay(100);
		m.Unlock();

		// Print(".");
	}
}

extern "C" void __cxa_pure_virtual()
{
	ASSERT(false, "Pure virtual function calles :(");
}

extern u32* _ctors_beg;
extern u32* _ctors_end;

#include"Video/VGA.hpp"

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");
	ASSERT(sizeof(size_t) == 4, "size_t");

	Terminal::Init();
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

	//Memory::Physical::PrintMemoryMap();

	Thread::Init();
	Timer::Init();
	Keyboard::Init();

	Print("=== VGA ===\n");
	VGA::Init();

	Print("3C2: %x\n", VGA::Read_3C2());
	//VGA::Write_3C2(0x68);
	//Print("3C2: %x\n", VGA::Read_3C2());

	VGA::Write_3C8(0, 32, 0, 32);
	VGA::Write_3C8(7, 255,255,255);

	union Test
	{
		u8 test;
		struct
		{
			u8 a : 1;
			u8 b : 2;
			u8 c : 5;
		};
	};

	Test t;
	t.a = 1;
	t.b = 1;
	t.c = 0;
	Print("Test: %x\n", t.test);

	VGA::HorizontalTiming ht;
	ht.Read();
	VGA::VerticalTiming vt;
	vt.Read();

	Print("=== Horizontal ===\n");
	Print("Total: %d\n", ht.total);
	Print("DisplayEnd: %d\n", ht.displayEnd);
	Print("BlankingStart: %d\n", ht.blankingStart);
	Print("DisplaySkew: %d\n", ht.displaySkew);
	Print("BlankingEnd: %d\n", ht.blankingEnd);
	Print("RetraceStart: %d\n", ht.retraceStart);
	Print("RetraceEnd: %d\n", ht.retraceEnd);
	Print("=== Vertical ===\n");
	Print("Total: %d\n", vt.total);
	Print("DisplayEnd: %d\n", vt.displayEnd);
	Print("BlankingStart: %d\n", vt.blankingStart);
	Print("BlankingEnd: %d\n", vt.blankingEnd);
	Print("RetraceStart: %d\n", vt.retraceStart);
	Print("RetraceEnd: %d\n", vt.retraceEnd);

	ht.displayEnd = 100;
	//ht.Write();
	vt.displayEnd = 600;
	//vt.Write();

	VGA::RWLogic rwLogic;
	rwLogic.Read();

	rwLogic.readMode = VGA::RWLogic::RM_0;
	rwLogic.writeMode = VGA::RWLogic::WM_0;
	rwLogic.memoryPlaneWriteEnable[0] = false;
	rwLogic.memoryPlaneWriteEnable[1] = false;
	rwLogic.memoryPlaneWriteEnable[2] = false;
	rwLogic.memoryPlaneWriteEnable[3] = false;

	rwLogic.logicalOperation = VGA::RWLogic::LO_XOR;
	rwLogic.rotateCount = 3;
	rwLogic.bitMask = 0x7f;

	//rwLogic.Write();

	VGA::GraphicsMode gm;
	gm.Read();
	gm.shiftColor256 = true;
	gm.shiftInterleaved = false;
	gm.Write();

	// Disable cursor
	VGA::Write_3D4(0xA, 0b00100000);
	// http://www.osdever.net/FreeVGA/vga/vga.htm#register
	// https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
	// http://xkr47.outerspace.dyndns.org/progs/mode%2013h%20without%20using%20bios.htm
	// https://01.org/sites/default/files/documentation/ilk_ihd_os_vol3_part1r2_0.pdf
	// https://www.amazon.com/dp/0201624907
	//VGA::Write_3C0(0x10, 0x41);
	//VGA::Write_3D4(0x17, 0xA3);

	__asm("cli");
	u8* vPtr = (u8*)(0x800a0000);
	for(unsigned a = 0; a < 320*200; a++)
	{
		vPtr[a] = a & 0xff;
	}

	if(!false)
	for(;;)
	{
		__asm("cli\nhlt");
	}

	Thread::Thread* testThread;
	Thread::Create(&testThread, (u8*)"YoLo", YoLo);

	// Interrupt::Enable();

	ISA_DMA::Init();

	/*Timer::Delay(1000);

	// Reset
	HAL::Out8(0x3c4, 0x00);
	HAL::Out8(0x3c5, 0x00);

	HAL::Out8(0x3c4, 0x01);
	HAL::Out8(0x3c5, (1 << 5));

	Timer::Delay(1000);

	HAL::Out8(0x3c4, 0x01);
	HAL::Out8(0x3c5, 0x00);

	// Un-reset
	HAL::Out8(0x3c4, 0x00);
	HAL::Out8(0x3c5, 0x03);*/

	/*for(;;)
	{
		HALT;
	}*/

	Block::Init();
	Block::Dummy::Init();
	Floppy::Init();
	ATA::Init();

	FS::Init();
	FAT::Init();
	FS::BlkFS::Init();

	VFS::Init();

	if(VFS::Mount("fdd0r1", "fdd") != Status::Success)
	{
		Print("No floppy :<\n");
		for(;;);
	}

	VFS::Mount("hdd0p3", "hdd");

	FS::Directory* dir;
	VFS::OpenRoot(&dir);

	Thread::Start(testThread);

	Thread::Thread* testThread2;
	Thread::Create(&testThread2, (u8*)"YoLo2", YoLo2);
	Thread::Start(testThread2);

#if 0
	{
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

#if 1
	char tmp[64];
	u8 tmpX = 0;
	Keyboard::KeyEvent keyEvent;
	Print("=== If you need help, write \"help\" ===\n");
	Print("> ");
	shellReady = true;
	for(;;)
	{
		while(Keyboard::WaitAndReadEvent(&keyEvent))
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
					tmpX = 0;
					Print("\nExecuting: %s\n", tmp);

					if(!strcmp(tmp, "help"))
					{
						Terminal::Print("=== Available commands:               ===\n");
						Terminal::Print("=== dir - lists current directory     ===\n");
						Terminal::Print("=== cd <x> - enters <x> directory     ===\n");
						Terminal::Print("=== cat <x> - reads <x> file          ===\n");
						Terminal::Print("=== mem - prints current memory usage ===\n");
						Terminal::Print("=== fail - kernel panic               ===\n");
						Terminal::Print("\n");
					}
					else if(!strcmp(tmp, "mem"))
					{
						Memory::Physical::PrintMemoryMap();
					}
					else if(!strcmp(tmp, "fail"))
					{
						u8* x = (u8*)0x1234;
						*x = 5;
					}
					else if(!strcmp(tmp, "task"))
					{
						Thread::UpdateThreadsList();
					}
					else if(!strcmp(tmp, "dir"))
					{
						//bd->drv->Lock(bd->dev);

						FS::DirEntry entry;
						Print("Dir: %p\n", dir);
						VFS::RewindDirectory(dir);

						Print("Directory content:\n");
						while(VFS::ReadDirectory(dir, &entry) == Status::Success)
						{
							if(!entry.isValid)
								continue;

							Print("<DATE>\t<HOUR>\t%s\t%u\t%s\n", (entry.isDirectory) ? "DIR" : "", entry.size, entry.name);
						}

						VFS::RewindDirectory(dir);
						//bd->drv->Unlock(bd->dev);
					}
					else if(strlen(tmp) > 3 && tmp[0] == 'c' && tmp[1] == 'd' && tmp[2] == ' ')
					{
						u8 path[256];
						u8* dst = path;
						bool foundAny = false;
						for(unsigned a = 3; a < strlen(tmp); a++)
						{
							if(tmp[a] != ' ')
							{
								//(*dst++) = tolower(tmp[a]);
								(*dst++) = tmp[a];
								foundAny = true;
							}
							else if(foundAny)
								break;
						}

						(*dst) = 0;

						if(VFS::ChangeDirectory(dir, path) == Status::Success)
							Print("Changed to: %s\n", path);
						else
							Print("Directory \"%s\" not found!\n", path);
					}
					else if(strlen(tmp) > 4 && tmp[0] == 'c' && tmp[1] == 'a' && tmp[2] == 't' && tmp[3] == ' ')
					{
						u8 path[256];
						u8* dst = path;
						bool foundAny = false;
						for(unsigned a = 4; a < strlen(tmp); a++)
						{
							if(tmp[a] != ' ')
							{
								//(*dst++) = tolower(tmp[a]);
								(*dst++) = tmp[a];
								foundAny = true;
							}
							else if(foundAny)
								break;
						}

						(*dst) = 0;

						FS::DirEntry entry;
						VFS::RewindDirectory(dir);

						FS::File* file = nullptr;
						while(VFS::ReadDirectory(dir, &entry) == Status::Success)
						{
							if(!entry.isValid || entry.isDirectory)
								continue;

							if(!strcmp((char*)path, (char*)entry.name))
							{
								VFS::OpenFile(dir, entry.name, &file);
								break;
							}
						}

						if(file)
							Print("Found file: %s\n", path);
						else
						{
							Print("File \"%s\" not found!\n", path);
							Print("\n> ");
							continue;
						}

						u32 readCount;
						const u32 bufSize = 512;
						u8 buf[bufSize];
						Status s;
						//bd->drv->Lock(bd->dev);
						while((s = VFS::ReadFile(file, buf, bufSize, &readCount)) == Status::Success)
						{
							for(unsigned a = 0; a < readCount; a++)
							{
								Print("%c", buf[a]);
							}
						}

						//bd->drv->Unlock(bd->dev);
						VFS::CloseFile(&file);
					}
					else
					{
						Print("Invalid command...\n");
					}
				}

				Print("\n");

				Path dirPath;
				VFS::GetPath(dir, dirPath);
				char tmp[Path::MaxLength];
				dirPath.ToString(tmp);

				Print("%s> ", tmp);
			}
			else if(keyEvent.ascii)
			{
				tmp[tmpX] = keyEvent.ascii;
				tmpX++;
				Print("%c", keyEvent.ascii);
			}
		}

		//__asm("sti");
		//__asm("hlt");
		//Timer::Delay(100);
	}
#endif

	for(;;)
	{
		__asm("hlt");
	}
}