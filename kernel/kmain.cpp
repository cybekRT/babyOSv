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

u8 tolower(u8 c);

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

	FS::Directory* dir;
	fs->OpenRoot(fsPriv, &dir);

	/*{
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
	}*/

	//for(;;);

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
					Print("\nExecuting: %s\n", tmp);

					if(strcmp(tmp, "mem"))
					{
						Memory::PrintMemoryMap();
					}
					else if(strcmp(tmp, "fail"))
					{
						u8* x = (u8*)0x1234;
						*x = 5;
					}
					else if(strcmp(tmp, "r"))
					{
						FS::DirEntry entry;
						fs->ReadDirectory(fsPriv, dir, &entry);

						Print("Entry (%x): %s\n", entry.isDirectory, entry.name);
					}
					else if(strcmp(tmp, "c"))
					{
						FS::DirEntry* entry = nullptr;
						fs->ChangeDirectory(fsPriv, dir);

						//Print("Changed to: (%x): %s\n", entry->type, entry->name);
					}
					else if(strcmp(tmp, "dir"))
					{
						dev->Lock(dev);

						FS::DirEntry entry;
						fs->RewindDirectory(fsPriv, dir);

						Print("Directory content:\n");
						while(fs->ReadDirectory(fsPriv, dir, &entry) != FS::Status::EOF)
						{
							if(!entry.isValid)
								continue;

							Print("<DATE>\t<HOUR>\t%s\t%u\t%s\n", (entry.isDirectory) ? "DIR" : "", entry.size, entry.name);
						}

						fs->RewindDirectory(fsPriv, dir);
						dev->Unlock(dev);
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
								(*dst++) = tolower(tmp[a]);
								foundAny = true;
							}
							else if(foundAny)
								break;
						}

						(*dst) = 0;

						FS::DirEntry entry;
						fs->RewindDirectory(fsPriv, dir);

						bool changed = false;
						while(fs->ReadDirectory(fsPriv, dir, &entry) != FS::Status::EOF)
						{
							if(!entry.isValid)
								continue;

							if(strcmp((char*)path, (char*)entry.name))
							{
								if(fs->ChangeDirectory(fsPriv, dir) == FS::Status::Success)
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
								(*dst++) = tolower(tmp[a]);
								foundAny = true;
							}
							else if(foundAny)
								break;
						}

						(*dst) = 0;

						FS::DirEntry entry;
						fs->RewindDirectory(fsPriv, dir);

						FS::File* file = nullptr;
						while(fs->ReadDirectory(fsPriv, dir, &entry) != FS::Status::EOF)
						{
							if(!entry.isValid || entry.isDirectory)
								continue;

							if(strcmp((char*)path, (char*)entry.name))
							{
								fs->OpenFile(fs, dir, &file);
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
						FS::Status s;
						while((s = fs->ReadFile(fsPriv, file, buf, bufSize, &readCount)) == FS::Status::Success)
						{
							for(unsigned a = 0; a < readCount; a++)
							{
								Print("%c", buf[a]);
							}
						}

						fs->CloseFile(fs, &file);
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