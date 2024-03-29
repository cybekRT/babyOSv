#include"VFS.hpp"
#include"Containers/List.hpp"
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

	Status Mount(char* partName, char* mountPoint, char* fsType)
	{
		auto parts = Block::GetPartitions();
		Block::BlockPartition* part = nullptr;

		Print("Partitions:\n");
		for(auto itr : parts)
		{
			Print("- %s\n", itr->name);
			if(!strcmp((char*)itr->name, partName))
			{
				part = itr;
				break;
			}
		}

		if(!part)
		{
			Print("No partition...\n");
			return Status::Undefined;
		}

		auto fss = FS::filesystems;
		FS::FSInfo* fsInfo = nullptr;
		for(auto itr : fss)
		{
			if(fsType && strcmp(fsType, itr->name))
				continue;

			if(itr->Probe(part) == Status::Success)
			{
				fsInfo = itr;
				break;
			}
		}

		if(!fsInfo)
		{
			Print("No filesystem...\n");
			return Status::Undefined;
		}

		void* fsPriv;
		if(fsInfo->Mount(part, &fsPriv) != Status::Success)
			return Status::Undefined;

		MountPoint mp = {
			.fsInfo = fsInfo,
			.fsPriv = fsPriv,
		};

		strcpy(mp.name, mountPoint);
		mountPoints.PushBack(mp);

		return Status::Success;
	}

	Status Unmount(char* mountPoint)
	{
		return Status::Undefined;
	}

	Status Flush()
	{
		for(auto itr : mountPoints)
		{
			itr.fsInfo->Flush(itr.fsPriv);
		}

		return Status::Success;
	}

	Status DirectoryOpenRoot(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid pointer to dir");

		//for(;;);
		//(*dir) = (FS::Directory*)Memory::Alloc(sizeof(FS::Directory));
		(*dir) = new FS::Directory();
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

	Status DirectoryClose(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid poitner to dir");

		if((*dir)->fsDir != nullptr)
		{
			(*dir)->fsInfo->DirectoryClose((*dir)->fsPriv, &(*dir)->fsDir);
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

	Status DirectoryRead(FS::Directory* dir, FS::DirEntry* entry)
	{
		ASSERT(dir, "No dir");
		ASSERT(entry, "No entry");

		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->DirectoryRead(dir->fsPriv, dir->fsDir, entry);
			return fsResult;
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

		return Status::Undefined;
	}

	Status DirectoryRewind(FS::Directory* dir)
	{
		ASSERT(dir, "No dir");

		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->DirectoryRewind(dir->fsPriv, dir->fsDir);
			return fsResult;
		}

		dir->index = -1;
		return Status::Success;
	}

	Status DirectoryChange(FS::Directory* dir, char* name)
	{
		ASSERT(dir, "No dir");

		Status status;
		Print("DirectoryChange(_, %s)\n", name);

		if(dir->fsInfo != nullptr)
		{
			Print("fsInfo != nullptr\n");
			status = Status::EndOfFile;
			dir->fsInfo->DirectoryRewind(dir->fsPriv, dir->fsDir);
			FS::DirEntry entry;
			while(dir->fsInfo->DirectoryRead(dir->fsPriv, dir->fsDir, &entry) == Status::Success)
			{
				if(!entry.isValid)
					continue;

				Print("Compare: \"%s\" with \"%s\": ", (char*)name, (char*)entry.name);
				if(!strcmp((char*)entry.name, (char*)name))
				{
					Print("Same!\n");
					status = dir->fsInfo->DirectoryFollow(dir->fsPriv, dir->fsDir);

					if(status == Status::Success)
					{
						if(strcmp((char*)entry.name, ".."))
						{
							dir->path.Add((char*)entry.name);
						}
						else
							dir->path.GoUp();
					}
				}
				else
					Print("Different!\n");
			}

			if(status != Status::Success && strcmp((char*)name, "..") == 0)
			{
				dir->fsInfo->DirectoryClose(dir->fsPriv, &dir->fsDir);

				dir->fsDir = nullptr;
				dir->fsInfo = nullptr;
				dir->fsPriv = nullptr;

				dir->path.GoUp();

				return Status::Success;
			}

			return status;
		}
		else
		{
			Print("fsInfo == nullptr\n");
			MountPoint* mp = nullptr;
			for(auto& itr : mountPoints)
			{
				Print("Compare: \"%s\" with \"%s\": ", (char*)name, (char*)itr.name);
				if(!strcmp((char*)name, itr.name))
				{
					Print("Same!\n");
					mp = &itr;
					break;
				}
				else
					Print("Different!\n");
			}

			if(!mp)
			{
				Print("No mount point~!\n");
				return Status::Undefined;
			}

			dir->fsInfo = mp->fsInfo;
			dir->fsPriv = mp->fsPriv;
			dir->fsInfo->DirectoryOpenRoot(dir->fsPriv, &dir->fsDir);

			dir->path.Add((char*)mp->name);
		}

		return Status::Success;
	}

	Status DirectoryCreate(FS::Directory* dir, char* name)
	{
		if(dir->fsInfo != nullptr)
		{
			return dir->fsInfo->DirectoryCreate(dir->fsPriv, dir->fsDir, name);
		}

		Print("Invalid path...\n");
		return Status::Undefined;
	}

	Status DirectoryRemove(FS::Directory* dir, char* name)
	{
		return Status::Undefined;
	}

	Status DirectoryGetPath(FS::Directory* dir, Path& path)
	{
		path = dir->path;
		return Status::Success;
	}

	Status FileCreate(FS::Directory* dir, char* name)
	{
		ASSERT(dir, "No directory");

		auto fsResult = dir->fsInfo->FileCreate(dir->fsPriv, dir->fsDir, name);

		return fsResult;
	}

	Status FileDelete(FS::Directory* dir, char* name)
	{
		return Status::Undefined;
	}

	Status FileOpen(FS::Directory* dir, char* name, FS::File** file)
	{
		ASSERT(dir, "No dir");
		ASSERT(file, "Invalid pointer to file");

		if(dir->fsInfo != nullptr)
		{
			dir->fsInfo->DirectoryRewind(dir->fsPriv, dir->fsDir);

			FS::DirEntry entry;
			while(dir->fsInfo->DirectoryRead(dir->fsPriv, dir->fsDir, &entry) == Status::Success)
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

			auto fsResult = dir->fsInfo->FileOpen(dir->fsPriv, dir->fsDir, &(*file)->fsFile);
			return fsResult;
		}

		return Status::Undefined;
	}

	Status FileOpen(char* pathStr, FS::File** file)
	{
		Path paths = Path(String(pathStr));

		FS::Directory* dir;
		DirectoryOpenRoot(&dir);

		Print("Path: %p\n", paths.paths.Back().Data());
		auto filename = paths.paths.Back();// paths.paths[2];
		paths.GoUp();

		// TODO: implement me
		for(auto path : paths.paths)
		{
			Print("Directory: %s\n", path.Data());
			auto status = VFS::DirectoryChange(dir, path.Data());
			if(status != Status::Success)
			{
				Print("Couldn't change to: %s\n", path.Data());
			}
		}

		// FS::File* f = nullptr;
		auto fopensts = FileOpen(dir, filename.Data(), file);
		Print("File open status: %d\n", fopensts);
		DirectoryClose(&dir);

		Print("Success~!\n");
		return Status::Success;
	}

	Status FileClose(FS::File** file)
	{
		ASSERT(file, "Pointer to file is invalid");

		(*file)->fsInfo->FileClose((*file)->fsPriv, &(*file)->fsFile);
		//Memory::Free(*file);
		delete *file;
		(*file) = nullptr;

		return Status::Success;
	}

	Status FileRead(FS::File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{
		ASSERT(file, "No file");

		auto fsResult = file->fsInfo->FileRead(file->fsPriv, file->fsFile, buffer, bufferSize, readCount);

		return fsResult;
	}

	Status FileWrite(FS::File* file, u8* buffer, u32 bufferSize, u32* writeCount)
	{
		ASSERT(file, "No file");

		auto fsResult = file->fsInfo->FileWrite(file->fsPriv, file->fsFile, buffer, bufferSize, writeCount);

		return fsResult;
	}

	Status FileSetPointer(FS::File* file, u32 offset)
	{
		ASSERT(file, "No file");

		auto fsResult = file->fsInfo->FileSetPointer(file->fsPriv, file->fsFile, offset);

		return fsResult;
	}

	Status FileGetPointer(FS::File* file, u32* offset)
	{
		ASSERT(file, "No file");

		auto fsResult = file->fsInfo->FileGetPointer(file->fsPriv, file->fsFile, offset);

		return fsResult;
	}
}