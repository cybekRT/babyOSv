#include"Terminal.h"
#include"Memory.h"
#include"Interrupt.h"
#include"Timer.h"
#include"Input/Keyboard.h"
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

bool shellReady = false;
u8 tolower(u8 c);

int YoLo(void*)
{
	for(;;)
	{
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

		Timer::Delay(500);
	}

	return 0;
}

int YoLo2(void*)
{
	for(;;)
	{
		Interrupt::Disable();

		((u8*)0x800b8000)[79*2+0]++;// = 0x00;

		Interrupt::Enable();
		//Print(".");
		Timer::Delay(100);
	}
}

extern "C" void __cxa_pure_virtual()
{
	ASSERT(false, "Pure virtual function calles :(");
}

extern u32* _ctors_beg;
extern u32* _ctors_end;

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");
	ASSERT(sizeof(size_t) == 4, "u32");

	Terminal::Init();
	Memory::Init();
	Interrupt::Init();

	/* Call global constructors */
	//Print("Constructors: (%p - %p)\n", _ctors_beg, _ctors_end);
	/*for(u32* ptr = _ctors_beg; ptr != _ctors_end; ptr++)
	{
		void (*func)() = (void (*)())(*ptr);
		Print("- %p\n", func);
		ASSERT(func, "Invalid constructor function~!");
		if(func)
			func();
	}*/

	//Memory::Physical::PrintMemoryMap();

	Thread::Init();
	Timer::Init();
	Keyboard::Init();

	Thread::Thread* testThread;
	Thread::Create(&testThread, (u8*)"YoLo", YoLo);

	Interrupt::Enable();

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