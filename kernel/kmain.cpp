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

class Test
{
	static int xd;

	public:
		LinkedList<int*> arr;

	Test()
	{
		Print("%s()\n", __FUNCTION__);
		arr.PushBack(new int(xd++));
		arr.PushBack(new int(xd++));
		arr.PushBack(new int(xd++));

		Print("Values:\n");
		for(auto itr : arr)
		{
			Print("- %d\n", *itr);
		}
	}

	~Test()
	{
		Print("%s()\n", __FUNCTION__);

		Print("Values:\n");
		for(auto itr : arr)
		{
			Print("- %d\n", *itr);
			*itr = -666;
			delete itr;
		}
	}
};

int Test::xd = 0;

int strlen(const char* str)
{
	unsigned len = 0;
	while(*str++)
		len++;

	return len;
}

int strcmp(char* a, char* b)
{
	while(*a || *b)
	{
		if(*a != *b)
			return (*a - *b);

		a++;
		b++;
	}

	return 0;
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

int strcat(const char* src, char* dst)
{
	int len = 0;

	while(*dst)
	{
		dst++;
	}

	while(*src)
	{
		*dst++ = *src++;
		len++;
	}

	*dst = 0;
	return len;
}

// Required if -O3 is used
extern "C"
{
	void* memmove(void* destination, const void* source, unsigned num)
	{
		// FIXME for overlapping buffers
		u8* src = (u8*)source;
		u8* dst = (u8*)destination;
		for(u32 a = 0; a < num; a++)
		{
			*dst++ = *src++;
		}

		return destination;
	}
}

u8 tolower(u8 c);

namespace Interrupt
{
	void Print();
}

void YoLo()
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

		Interrupt::Print();

		Thread::SetState(nullptr, Thread::State::Running);
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

extern "C" void __cxa_pure_virtual()
{
	ASSERT(false, "Pure virtual function calles :(");
}

//extern void (*__ctors_beg)();
//extern void (*__ctors_end)();
extern u32* _ctors_beg;
extern u32* _ctors_end;

extern "C" void kmain()
{
	ASSERT(sizeof(u64) == 8, "u64");
	ASSERT(sizeof(u32) == 4, "u32");
	ASSERT(sizeof(u16) == 2, "u16");
	ASSERT(sizeof(u8) == 1, "u8");

	Terminal::Init();
	Memory::Init();
	Interrupt::Init();

	Print("Ctors beg: %p\n", _ctors_beg);
	Print("Ctors end: %p\n", _ctors_end);
	for(u32* ptr = _ctors_beg; ptr != _ctors_end; ptr++)
	{
		Print("Pointer: %p\n", ptr);
		void (*func)() = (void (*)())(*ptr);
		Print("Calling: %p\n", func);
		if(func)
			func();
		// FIXME~!
	}

	//for(;;);

	Thread::Init();
	Timer::Init();
	Keyboard::Init();

	Thread::Thread* testThread;
	Thread::Thread* halterThread;
	Thread::Create(&testThread, YoLo, (u8*)"YoLo");
	Thread::Create(&halterThread, Halter, (u8*)"Halter");

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

	//Print("Mounting floppy...\n");

	FS::Directory* dir;
	VFS::OpenRoot(&dir);

	/*auto t1 = new Test();
	auto t2 = new Test();
	Test t3(*t2);
	delete t2;
	//for(;;);*/

	//t3.~Test();

	/*Print("====================\n");
	{
		Path p;
		Print(">===================\n");
		p.Add("YoLo");
		Print("<===================\n");
	}*/

	//for(;;);

	/*for(;;)
	{
		break;
		Timer::Delay(-1);
	}*/

	Print("==========\n");
	Path* p = new Path();
	Print("Path pointer: %p\n", p);

	Array<int> array;
	array.PushBack(5);
	array.PushBack(1);
	array.PushBack(2);
	array.PushBack(59873);

	auto itr = array.begin() + 0; //++itr; ++itr;
	array.Insert(itr, 666);

	for(auto itr : array)
	{
		Print("Value: %d\n", itr);
	}

#if 0
	for(;;)
	{
		for(auto part : Block::GetPartitions())
		{
			Print("\n\nPartition: %s\n\n", part.name);
			Timer::Delay(200);
			for(auto fsItem = FS::filesystems.data; fsItem != nullptr; fsItem = fsItem->next)
			{
				auto fs = fsItem->value;
				if(fs->Probe(&part) != FS::Status::Success)
					continue;

				void* fsPriv;
				fs->Alloc(&part, &fsPriv);

				FS::Directory* dir;
				fs->OpenRoot(fsPriv, &dir);

				Print("Root:\n");
				FS::DirEntry entry;
				while(fs->ReadDirectory(fsPriv, dir, &entry) == FS::Status::Success)
				{
					if(!entry.isValid)
						continue;

					Print("- %s\n", entry.name);
				}

				fs->CloseDirectory(fsPriv, &dir);
				fs->Dealloc(&part, &fsPriv);
				Timer::Delay(1000);
				break;
			}

			Memory::Physical::PrintMemoryMap();
			Timer::Delay(1000);
		}
	}
#endif

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

						/*FS::DirEntry entry;
						VFS::RewindDirectory(dir);

						bool changed = false;
						while(VFS::ReadDirectory(dir, &entry) == Status::Success)
						{
							if(!entry.isValid)
								continue;

							if(!strcmp((char*)path, (char*)entry.name))
							{
								if(VFS::ChangeDirectory(dir, entry.name) == Status::Success)
									changed = true;
								break;
							}
						}*/

						//if(changed)
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

		__asm("sti");
		__asm("hlt");
	}
#endif

	for(;;)
	{
		__asm("hlt");
	}
}