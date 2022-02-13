#include"Input/Keyboard.hpp"
#include"Memory.hpp"
#include"Thread.hpp"
#include"FS/VFS.hpp"
#include"Video/VGA.hpp"

namespace Shell
{
	int Thread(void* args)
	{
		FS::Directory* dir;
		VFS::DirectoryOpenRoot(&dir);

		char tmp[64];
		u8 tmpX = 0;
		Keyboard::KeyEvent keyEvent;
		Print("=== If you need help, write \"help\" ===\n");
		Print("> ");

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
							// FIXME: "cd ." causes infinite loop...
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
						else if(strlen(tmp) > 4 && tmp[0] == 'm' && tmp[1] == 'k' && tmp[2] == 'd' && tmp[3] == ' ')
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

							auto s = VFS::DirectoryCreate(dir, (char*)path);
							Print("Status: %d - \"%s\"\n", s, path);
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
	}
}