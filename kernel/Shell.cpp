#include"Input/Keyboard.hpp"
#include"Memory.hpp"
#include"Thread.hpp"
#include"FS/VFS.hpp"
#include"Video/VGA.hpp"
#include"Containers/Array.hpp"
#include"Containers/List.hpp"

namespace Shell
{
	FS::Directory* dir;

	typedef int(*HandlerPtr)(int, char**);
	struct HandlerDef
	{
		const char* name;
		HandlerPtr handler;
		const char* description;
	};
	List<HandlerDef> handlers;

	class RegHandler
	{
		public:
			RegHandler(const char* name, HandlerPtr handler, const char* description = nullptr)
			{
				handlers.PushBack( { .name = name, .handler = handler, .description = description } );
			}
	};

	#define HANDLER(name, ...) int Shell_##name(int argc, char** argv); RegHandler regHandler_##name(#name, Shell_##name __VA_OPT__(,) __VA_ARGS__); int Shell_##name(int argc, char** argv)

	HANDLER(help, "Print help")
	{
		Print("=== Available commands ===\n");

		unsigned maxLen = 0;
		for(auto f : handlers)
		{
			unsigned len = strlen(f.name);
			if(len > maxLen)
				maxLen = len;
		}

		for(auto f : handlers)
		{
			Terminal::PrintWithPadding(maxLen + 2, "- %s", f.name);
			if(f.description)
				Print(" - %s", f.description);
			Print("\n");
		}

		return 0;
	}

	HANDLER(mem, "Print memory map")
	{
		Memory::Physical::PrintMemoryMap();
		return 0;
	}

	HANDLER(test, "Test file")
	{
		FS::Directory* dir;
		VFS::DirectoryOpenRoot(&dir);
		VFS::DirectoryChange(dir, "fdd");

		char tmp[] = "YoLo YoLo YoLo YoLo YoLo";

		FS::File* file;
		u32 wc;
		VFS::FileCreate(dir, "yolo.txt");
		VFS::FileOpen(dir, "yolo.txt", &file);
		VFS::FileWrite(file, (u8*)tmp, sizeof(tmp), &wc);
		Print("Written: %d\n", wc);
		VFS::FileClose(&file);

		return 0;
	}

	HANDLER(fail, "Crash system")
	{
		u8* x = (u8*)0x1234;
		*x = 5;
	}

	// HANDLER(task, "Print tasks list")
	// {
	// 	Thread::UpdateThreadsList();
	// 	return 0;
	// }

	HANDLER(dir, "List current directory")
	{
		FS::DirEntry entry;
		VFS::DirectoryRewind(dir);

		Print("Directory content:\n");
		while(VFS::DirectoryRead(dir, &entry) == Status::Success)
		{
			if(!entry.isValid)
				continue;

			Print("<DATE>\t<HOUR>\t%s\t%u\t%s\n", (entry.isDirectory) ? "DIR" : "", entry.size, entry.name);
		}

		VFS::DirectoryRewind(dir);

		return 0;
	}

	HANDLER(cd, "Change directory")
	{
		// FIXME: "cd ." causes infinite loop...
		if(argc < 2)
			return 1;

		char* path = argv[1];

		if(VFS::DirectoryChange(dir, path) != Status::Success)
		{
			Print("Directory \"%s\" not found!\n", path);
			return 1;
		}

		Print("Changed to: %s\n", path);
		return 0;
	}

	HANDLER(cat, "Read file and print contents")
	{
		if(argc < 2)
			return 1;

		char* path = argv[1];

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

		if(!file)
		{
			Print("File \"%s\" not found!\n", path);
			return 1;
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

		return 0;
	}

	HANDLER(mkdir, "Create directory")
	{
		if(argc < 2)
			return 1;

		char* path = argv[1];

		auto s = VFS::DirectoryCreate(dir, (char*)path);
		Print("Status: %d - \"%s\"\n", s, path);
	}

	HANDLER(splash, "Read and display splash")
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

	Array<char*> SplitParameters(char* buffer, u32 bufferSize)
	{
		Array<char*> params;
		Print("Buffer: %p, size: %d\n", buffer, bufferSize);

		bool isWhitespace = true;
		bool isQuoted = false;
		for(unsigned a = 0; a < bufferSize; a++)
		{
			if(buffer[a] == '\0')
				break;

			if(buffer[a] == '"')
			{
				if(a == 0 || buffer[a - 1] != '\\')
				{
					isQuoted = !isQuoted;

					if(!isQuoted)
						buffer[a] = ' ';
					else
					{
						isWhitespace = false;
						params.PushBack(&buffer[a + 1]);
						continue;
					}
				}
			}
			else if(buffer[a] == '\\' && a + 1 < bufferSize && buffer[a + 1] == '"')
				continue;

			if(isWhitespace && buffer[a] != ' ')
			{
				isWhitespace = false;
				params.PushBack(&buffer[a]);
			}
			else if(!isWhitespace && buffer[a] == ' ')
			{
				if(isQuoted)
					continue;

				isWhitespace = true;
				buffer[a] = 0;
			}
		}

		return params;
	}

	int Thread(void* args)
	{
		VFS::DirectoryOpenRoot(&dir);

		Keyboard::KeyEvent keyEvent;
		Array<char> buffer;
		Path dirPath;

		bool printPrompt = true;
		for(;;)
		{
			if(printPrompt)
			{
				VFS::Flush();

				VFS::DirectoryGetPath(dir, dirPath);
				char tmp[Path::MaxLength];
				dirPath.ToString(tmp);

				Print("\n%s> ", tmp);
				printPrompt = false;
			}

			if(Keyboard::WaitAndReadEvent(&keyEvent))
			{
				if(keyEvent.type == Keyboard::KeyType::Released)
					continue;

				if(keyEvent.key == Keyboard::KeyCode::Backspace)
				{
					if(!buffer.IsEmpty())
					{
						buffer.PopBack();
						Print("\b");
					}
				}
				else if(keyEvent.key == Keyboard::KeyCode::Enter)
				{
					if(buffer.IsEmpty())
						continue;

					buffer.PushBack(0); // Terminate string
					Print("Created array\n");
					Array<char*> params;
					Print("Splitting params...\n");
					params = SplitParameters(&buffer[0], buffer.Size());

					if(!params.IsEmpty())
					{
						for(auto func : handlers)
						{
							if(strcmp(func.name, params[0]) == 0)
							{
								Print("Executing: \"%s\" (%d)\n", func.name, params.Size());
								auto res = func.handler(params.Size(), &params[0]);
								if(res)
									Print("Function failed with error code: %d\n", res);

								break;
							}
						}
					}

					printPrompt = true;
					buffer.Clear();
				}
				else if(keyEvent.ascii)
				{
					buffer.PushBack(keyEvent.ascii);
					Print("%c", keyEvent.ascii);
				}
				else
				{
					// INV key
				}
			}
		}
	}
}