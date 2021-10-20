#include"VFS.hpp"
#include"Container/LinkedList.h"
#include"Path.hpp"

namespace FS
{
	struct File
	{
		FSInfo *fsInfo;
		void* fsPriv;
		FS::File* fsFile;
	};

	struct Directory
	{
		u32 index;

		FSInfo *fsInfo;
		void* fsPriv;
		FS::Directory* fsDir;
		//LinkedList<u8*> path;
		Path path;
	};
}

namespace VFS
{
	struct MountPoint
	{
		char		name[Path::MaxLength];
		FS::FSInfo*	fsInfo;
		void*		fsPriv;
	};

	Container::Array<MountPoint> mountPoints;

	Status Init()
	{
		return Status::Success;
	}

	Status Mount(char* partName, char* mountPoint)
	{
		auto parts = Block::GetPartitions();
		Block::BlockPartition* part = nullptr;

		for(auto itr : parts)
		{
			if(!strcmp((char*)itr->name, partName))
			{
				part = itr;
				break;
			}
		}

		if(!part)
		{
			Print("No partition...\n");
			return Status::Fail;
		}

		auto fss = FS::filesystems;
		FS::FSInfo* fsInfo = nullptr;
		for(auto itr : fss)
		{
			if(itr->Probe(part) == FS::Status::Success)
			{
				fsInfo = itr;
				break;
			}
		}

		if(!fsInfo)
		{
			Print("No filesystem...\n");
			return Status::Fail;
		}

		void* fsPriv;
		if(fsInfo->Alloc(part, &fsPriv) != FS::Status::Success)
			return Status::Fail;

		MountPoint mp = {
			.fsInfo = fsInfo,
			.fsPriv = fsPriv,
		};

		strcpy(mp.name, mountPoint);
		mountPoints.PushBack(mp);

		return Status::Success;
	}

	Status OpenRoot(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid pointer to dir");

		//for(;;);
		//(*dir) = (FS::Directory*)Memory::Alloc(sizeof(FS::Directory));
		Print("Opening root...\n");
		(*dir) = new FS::Directory();
		Print("Opened root: %p\n", (*dir));
		//new( (*dir)) FS::Directory();
		(*dir)->index = -1;

		(*dir)->fsInfo = nullptr;
		(*dir)->fsPriv = nullptr;
		(*dir)->fsDir = nullptr;
		//new( &(*dir)->path ) Path();
		//new Path((*dir)->path);
		//new(*dir) FS::Directory();
		// TODO: use new everywhere and call constructors~!!!

		return Status::Success;
	}

	Status OpenDirectory(FS::Directory* src, FS::Directory** dir)
	{
		return Status::Fail;
	}

	Status CloseDirectory(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid poitner to dir");

		if((*dir)->fsDir != nullptr)
		{
			(*dir)->fsInfo->CloseDirectory((*dir)->fsPriv, &(*dir)->fsDir);
		}

		/*while(!(*dir)->path.IsEmpty())
		{
			auto ptr = !(*dir)->path.PopFront();
			Memory::Free((void*)ptr);
		}*/

		//Memory::Free((void*)*dir);
		//Memory::Free(*dir);
		delete *dir;
		(*dir) = nullptr;
		return Status::Success;
	}

	Status RewindDirectory(FS::Directory* dir)
	{
		ASSERT(dir, "No dir");

		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->RewindDirectory(dir->fsPriv, dir->fsDir);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		dir->index = -1;
		return Status::Success;
	}

	Status ReadDirectory(FS::Directory* dir, FS::DirEntry* entry)
	{
		ASSERT(dir, "No dir");
		ASSERT(entry, "No entry");

		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->ReadDirectory(dir->fsPriv, dir->fsDir, entry);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		dir->index++;

		unsigned a = 0;
		for(auto mp : mountPoints)
		{
			if(a == dir->index)
			{
				entry->isValid = 1;
				entry->isDirectory = 1;
				entry->isHidden = 0;
				entry->isSymlink = 0;

				strcpy((char*)entry->name, mp.name);

				return Status::Success;
			}
			else if(a > dir->index)
				break;

			a++;
		}

		return Status::Fail;
	}

	Status ChangeDirectory(FS::Directory* dir, u8* name)
	{
		ASSERT(dir, "No dir");

		FS::Status status;

		if(dir->fsInfo != nullptr)
		{
			status = FS::Status::EOF;
			dir->fsInfo->RewindDirectory(dir->fsPriv, dir->fsDir);
			FS::DirEntry entry;
			Print("Reading directory...\n");
			while(dir->fsInfo->ReadDirectory(dir->fsPriv, dir->fsDir, &entry) == FS::Status::Success)
			{
				Print(">");
				if(!entry.isValid)
					continue;

				if(!strcmp((char*)entry.name, (char*)name))
				{
					status = dir->fsInfo->ChangeDirectory(dir->fsPriv, dir->fsDir);

					if(status == FS::Status::Success)
					{
						if(strcmp((char*)entry.name, ".."))
						{
							Print("Adding path: %s\n", entry.name);
							dir->path.Add((char*)entry.name);
						}
						else
							dir->path.GoUp();
					}
				}
			}

			if(status != FS::Status::Success && strcmp((char*)name, "..") == 0)
			{
				dir->fsInfo->CloseDirectory(dir->fsPriv, &dir->fsDir);

				dir->fsDir = nullptr;
				dir->fsInfo = nullptr;
				dir->fsPriv = nullptr;

				dir->path.GoUp();

				return Status::Success;
			}

			Print("Result %x: %s:%d\n", status, __FILE__, __LINE__);
			return (status == FS::Status::Success ? Status::Success : Status::Fail);
		}
		else
		{
			MountPoint* mp = nullptr;
			for(auto itr : mountPoints)
			{
				if(!strcmp((char*)name, itr.name))
				{
					mp = &itr;
					break;
				}
			}

			if(!mp)
			{
				return Status::Fail;
			}

			dir->fsInfo = mp->fsInfo;
			dir->fsPriv = mp->fsPriv;
			dir->fsInfo->OpenRoot(dir->fsPriv, &dir->fsDir);

			Print("Adding path: %s\n", mp->name);
			dir->path.Add((char*)mp->name);
		}

		Print("ChangeDirectory: success~!\n");
		return Status::Success;
	}

	Status GetPath(FS::Directory* dir, Path& path)
	{
		path = dir->path;
		return Status::Success;
	}

	Status IsRoot(FS::Directory* dir, bool* result)
	{

	}

	Status OpenFile(FS::Directory* dir, u8* name, FS::File** file)
	{
		ASSERT(dir, "No dir");
		ASSERT(file, "Invalid pointer to file");

		if(dir->fsInfo != nullptr)
		{
			dir->fsInfo->RewindDirectory(dir->fsPriv, dir->fsDir);

			FS::DirEntry entry;
			while(dir->fsInfo->ReadDirectory(dir->fsPriv, dir->fsDir, &entry) == FS::Status::Success)
			{
				if(!entry.isValid)
					continue;

				if(!strcmp((char*)entry.name, (char*)name))
					break;
			}

			//(*file) = (FS::File*)Memory::Alloc(sizeof(FS::File));
			(*file) = new FS::File();
			(*file)->fsInfo = dir->fsInfo;
			(*file)->fsPriv = dir->fsPriv;

			auto fsResult = dir->fsInfo->OpenFile(dir->fsPriv, dir->fsDir, &(*file)->fsFile);
			Print("Result %x: %s:%d\n", fsResult, __FILE__, __LINE__);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		return Status::Fail;
	}

	Status CloseFile(FS::File** file)
	{
		ASSERT(file, "Pointer to file is invalid");

		(*file)->fsInfo->CloseFile((*file)->fsPriv, &(*file)->fsFile);
		//Memory::Free(*file);
		delete *file;
		(*file) = nullptr;

		return Status::Success;
	}

	Status ReadFile(FS::File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{
		ASSERT(file, "No file");

		auto fsResult = file->fsInfo->ReadFile(file->fsPriv, file->fsFile, buffer, bufferSize, readCount);

		return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
	}
}