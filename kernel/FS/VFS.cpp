#include"VFS.hpp"
#include"Container/LinkedList.h"
#include"Path.hpp"

#include<new>

int strlen(const char* str);
int strcpy(const char* src, char* dst);

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

	Array<MountPoint> mountPoints;

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
			if(itr.value->Probe(part) == FS::Status::Success)
			{
				fsInfo = itr.value;
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

		strcpy(mountPoint, mp.name);
		mountPoints.PushBack(mp);

		return Status::Success;
	}

	Status OpenRoot(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid pointer to dir");

		//for(;;);
		(*dir) = (FS::Directory*)Memory::Alloc(sizeof(FS::Directory));
		//new( (*dir)) FS::Directory();
		(*dir)->index = -1;

		(*dir)->fsInfo = nullptr;
		(*dir)->fsPriv = nullptr;
		(*dir)->fsDir = nullptr;
		new( &(*dir)->path ) Path();
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
		Memory::Free(*dir);
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
		//for(const auto& itr : Block::GetDevices())
		Print("Checking...\n");
		for(const auto& itr : Block::GetPartitions())
		{
			//Print("Itr name: %s\n", itr->name);
			//Print("Entry name: %s\n", entry->name);
			//Print("a = %d, index = %d\n", a, dir->index);
			if(a == dir->index)
			{
				entry->isValid = 1;
				entry->isDirectory = 1;
				entry->isHidden = 0;
				entry->isSymlink = 0;

				//Print("Itr name: %s\n", itr->name);
				//Print("Entry name: %s\n", entry->name);
				strcpy((char*)itr->name, (char*)entry->name);

				return Status::Success;
			}
			else if(a > dir->index)
				break;

			a++;
		}

		//Print("[%d] last index... %d\n", a, dir->index);
		return Status::Fail;
	}

	Status ChangeDirectory(FS::Directory* dir, u8* name)
	{
		ASSERT(dir, "No dir");

		FS::Status status;

		if(dir->fsInfo != nullptr)
		{
			//status = dir->fsInfo->ChangeDirectory(dir->fsPriv, dir->fsDir);

			status = FS::Status::EOF;
			Print("FSInfo2: %x", dir->fsInfo);
			dir->fsInfo->RewindDirectory(dir->fsPriv, dir->fsDir);
			FS::DirEntry entry;
			Print("Reading directory...\n");
			Print(" %x\n", dir);
			Print(" %x\n", dir->fsInfo);
			Print(" %x\n", dir->fsInfo->ReadDirectory);
			while(dir->fsInfo->ReadDirectory(dir->fsPriv, dir->fsDir, &entry) == FS::Status::Success)
			{
				Print(">");
				if(!entry.isValid)
					continue;

				if(!strcmp((char*)entry.name, (char*)name))
				{
					//u8* pathBuffer = (u8*)Memory::Alloc(FS::MaxFilenameLength);
					//strcpy((char*)entry.name, (char*)pathBuffer);

					status = dir->fsInfo->ChangeDirectory(dir->fsPriv, dir->fsDir);

					if(status == FS::Status::Success)
					{
						//dir->path.PushBack(pathBuffer);
						if(!strcmp((char*)entry.name, ".."))
						{
							Print("Adding path: %s\n", entry.name);
							dir->path.Add((char*)entry.name);
						}
						else
							dir->path.GoUp();
					}
				}
			}

			Print("Result %x: %s:%d\n", status, __FILE__, __LINE__);
			//return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}
		else
		{
			//Block::BlockDevice* bd = nullptr;
			Block::BlockPartition* part = nullptr;

			unsigned a = 0;
			for(auto& itr : Block::GetPartitions())
			{
				if(a++ == dir->index)
				{
					//bd = &itr;
					part = itr;
					break;
				}
			}

			if(!part)
			{
				Print("Fail: %s:%d\n", __FILE__, __LINE__);
				return Status::Fail;
			}

			auto fsItr = FS::filesystems.data;
			while(fsItr)
			{
				if(fsItr->value->Probe(part) == FS::Status::Success)
					break;

				fsItr = fsItr->next;
			}

			if(!fsItr)
			{
				Print("Fail: %s:%d\n", __FILE__, __LINE__);
				return Status::Fail;
			}

			dir->fsInfo = fsItr->value;
			dir->fsInfo->Alloc(part, &dir->fsPriv);
			dir->fsInfo->OpenRoot(dir->fsPriv, &dir->fsDir);
			Print("FSInfo: %x\n", dir->fsInfo);

			dir->path.Add((char*)part->name);
		}

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

			(*file) = (FS::File*)Memory::Alloc(sizeof(FS::File));
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
		Memory::Free(*file);
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