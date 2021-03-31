#include"Terminal.h"
#include"Memory.h"
#include"Interrupt.h"
#include"Timer.h"
#include"Keyboard.h"
#include"Thread.hpp"

#include"ISA_DMA.hpp"

#include"Block/Block.hpp"
#include"Block/Floppy.hpp"

#include"FS/FS.hpp"
#include"FS/FS_FAT12.hpp"

#include"VFS.hpp"

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

int strcpy(const char* src, char* dst)
{
	int len = 0;
	while(*src)
	{
		*dst++ = *src++;
		len++;
	}

	*dst = 0;
	return len;
}

u8 tolower(u8 c);

void YoLo()
{
	u8* p = (u8*)(0x800B8001);
	for(;;)
	{
		//Terminal::Print("^");
		(*p)++;
		Timer::Delay(500);
	}
}

void Halter()
{
	for(;;)
	{
		__asm("hlt");
	}
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
	Thread::Init();
	Timer::Init();
	Keyboard::Init();

	Thread::Thread* testThread;
	Thread::Thread* halterThread;
	Thread::Create(&testThread, YoLo, (u8*)"YoLo");
	Thread::Create(&halterThread, Halter, (u8*)"Halter");

	Interrupt::Enable();
	PutString("Kernel halted~!\n");

	ISA_DMA::Init();

	Block::Init();
	Floppy::Init();

	FS::Init();
	FS_FAT12::Init();

	VFS::Init();

	Print("Mounting floppy...\n");

	auto bd = Block::devices.Front();
	auto fs = FS::filesystems.Front();
	void* fsPriv;

	fs->Alloc(bd, &fsPriv);

	FS::Directory* dir;
	VFS::OpenRoot(&dir);
	//fs->OpenRoot(fsPriv, &dir);

#if 0
	{
		Print("Searching splash\n");
		FS::DirEntry entry;
		while(fs->ReadDirectory(fsPriv, dir, &entry) == FS::Status::Success)
		{
			if(strcmp((char*)entry.name, "splash"))
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

	char tmp[64];
	u8 tmpX = 0;
	Keyboard::KeyEvent keyEvent;
	Print("=== If you need help, write \"help\" ===\n");
	Print("> ");
	for(;;)
	{
		//Print(".");
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
					Print("\nExecuting: %s\n", tmp);

					if(strcmp(tmp, "help"))
					{
						Terminal::Print("=== Available commands:               ===\n");
						Terminal::Print("=== dir - lists current directory     ===\n");
						Terminal::Print("=== cd <x> - enters <x> directory     ===\n");
						Terminal::Print("=== cat <x> - reads <x> file          ===\n");
						Terminal::Print("=== mem - prints current memory usage ===\n");
						Terminal::Print("=== fail - kernel panic               ===\n");
						Terminal::Print("\n");
					}
					else if(strcmp(tmp, "mem"))
					{
						Memory::PrintMemoryMap();
					}
					else if(strcmp(tmp, "fail"))
					{
						u8* x = (u8*)0x1234;
						*x = 5;
					}
					else if(strcmp(tmp, "dir"))
					{
						bd->drv->Lock(bd->dev);

						FS::DirEntry entry;
						VFS::RewindDirectory(dir);

						Print("Directory content:\n");
						while(VFS::ReadDirectory(dir, &entry) == Status::Success)
						{
							if(!entry.isValid)
								continue;

							Print("<DATE>\t<HOUR>\t%s\t%u\t%s\n", (entry.isDirectory) ? "DIR" : "", entry.size, entry.name);
						}

						VFS::RewindDirectory(dir);
						bd->drv->Unlock(bd->dev);
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

						FS::DirEntry entry;
						VFS::RewindDirectory(dir);

						bool changed = false;
						while(VFS::ReadDirectory(dir, &entry) == Status::Success)
						{
							if(!entry.isValid)
								continue;

							if(strcmp((char*)path, (char*)entry.name))
							{
								if(VFS::ChangeDirectory(dir) == Status::Success)
									changed = true;
								break;
							}
						}

						if(changed)
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

							if(strcmp((char*)path, (char*)entry.name))
							{
								VFS::OpenFile(dir, &file);
								break;
							}
						}

						if(file)
							Print("Found file: %s\n", path);
						else
							Print("File \"%s\" not found!\n", path);

						u32 readCount;
						const u32 bufSize = 512;
						u8 buf[bufSize];
						Status s;
						bd->drv->Lock(bd->dev);
						while((s = VFS::ReadFile(file, buf, bufSize, &readCount)) == Status::Success)
						{
							for(unsigned a = 0; a < readCount; a++)
							{
								Print("%c", buf[a]);
							}
						}

						bd->drv->Unlock(bd->dev);
						VFS::CloseFile(&file);
					}
					else
					{
						Print("Invalid command...\n");
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

	for(;;);
}