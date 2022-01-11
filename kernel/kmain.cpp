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

#include"Video/VGA.hpp"

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

	FS::Directory* dir;
	VFS::DirectoryOpenRoot(&dir);

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
	// Thread::Thread* fakeKbdThread;
	// Thread::Create(&fakeKbdThread, (u8*)"FakeKbd", FakeKbdThread);
	// Thread::Start(fakeKbdThread);

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
						VFS::DirectoryRewind(dir);

						Print("Directory content:\n");
						while(VFS::DirectoryRead(dir, &entry) == Status::Success)
						{
							if(!entry.isValid)
								continue;

							Print("<DATE>\t<HOUR>\t%s\t%u\t%s\n", (entry.isDirectory) ? "DIR" : "", entry.size, entry.name);
						}

						VFS::DirectoryRewind(dir);
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

						if(VFS::DirectoryChange(dir, (char*)path) == Status::Success)
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
						VFS::DirectoryRewind(dir);

						FS::File* file = nullptr;
						while(VFS::DirectoryRead(dir, &entry) == Status::Success)
						{
							if(!entry.isValid || entry.isDirectory)
								continue;

							if(!strcmp((char*)path, (char*)entry.name))
							{
								VFS::FileOpen(dir, (char*)entry.name, &file);
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
						while((s = VFS::FileRead(file, buf, bufSize, &readCount)) == Status::Success)
						{
							for(unsigned a = 0; a < readCount; a++)
							{
								Print("%c", buf[a]);
							}
						}

						//bd->drv->Unlock(bd->dev);
						VFS::FileClose(&file);
					}
					else if(strcmp(tmp, "splash") == 0)
					{
						FS::Directory* splashDir;
						FS::File* splashFile;
						u32 splashRead;
						u8* splashBuffer = new u8[320*200];
						VFS::DirectoryOpenRoot(&splashDir);
						VFS::DirectoryChange(splashDir, "fdd");
						VFS::FileOpen(splashDir, "splash", &splashFile);
						VFS::FileRead(splashFile, splashBuffer, 320*200, &splashRead);
						Print("Splash size: %d\n", splashRead);

						Print("=== VGA ===\n");
						VGA::Init();
						memcpy((void*)0x800a0000, splashBuffer, splashRead);

						for(unsigned a = 0; a < 256; a++)
						{
							int v = (a << 2);
							VGA::Write_3C8(a, v, v, v);
						}
					}
					else
					{
						Print("Invalid command...\n");
					}
				}

				Print("\n");

				Path dirPath;
				VFS::DirectoryGetPath(dir, dirPath);
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
#endif

	Print("\n\nKernel halted, should shutdown now...\n");
	for(;;)
	{
		__asm("cli");
		__asm("hlt");
	}
}