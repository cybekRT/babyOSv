#include"VFS.hpp"

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
	};
}

namespace VFS
{
	Status Init()
	{
		return Status::Success;
	}

	Status OpenRoot(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid poitner to dir");

		(*dir) = (FS::Directory*)Memory::Alloc(sizeof(FS::Directory));
		(*dir)->index = -1;

		(*dir)->fsInfo = nullptr;
		(*dir)->fsPriv = nullptr;
		(*dir)->fsDir = nullptr;
	}

	Status OpenDirectory(FS::Directory* src, FS::Directory** dir)
	{

	}

	Status CloseDirectory(FS::Directory** dir)
	{
		ASSERT(dir, "Invalid poitner to dir");

		if((*dir)->fsDir != nullptr)
		{
			(*dir)->fsInfo->CloseDirectory((*dir)->fsPriv, &(*dir)->fsDir);
		}

		Memory::Free((void*)*dir);
		(*dir) = nullptr;
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
		for(auto& itr : Block::devices)
		{
			if(a == dir->index)
			{
				entry->isValid = 1;
				entry->isDirectory = 1;
				entry->isHidden = 0;
				entry->isSymlink = 0;

				strcpy((char*)itr.name, (char*)entry->name);

				return Status::Success;
			}
			else if(a >= dir->index)
				break;
		}

		return Status::Fail;
	}

	Status ChangeDirectory(FS::Directory* dir)
	{
		ASSERT(dir, "No dir");

		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->ChangeDirectory(dir->fsPriv, dir->fsDir);
			Print("Result %x: %s:%d\n", fsResult, __FILE__, __LINE__);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		Block::BlockDevice* bd = nullptr;

		unsigned a = 0;
		for(auto& itr : Block::devices)
		{
			if(a == dir->index)
			{
				bd = &itr;
				break;
			}
		}

		if(!bd)
		{
			Print("Fail: %s:%d\n", __FILE__, __LINE__);
			return Status::Fail;
		}

		auto fsItr = FS::filesystems.data;
		while(fsItr)
		{
			if(fsItr->value->Probe(bd) == FS::Status::Success)
				break;

			fsItr = fsItr->next;
		}

		if(!fsItr)
		{
			Print("Fail: %s:%d\n", __FILE__, __LINE__);
			return Status::Fail;
		}

		dir->fsInfo = fsItr->value;
		dir->fsInfo->Alloc(bd, &dir->fsPriv);
		dir->fsInfo->OpenRoot(dir->fsPriv, &dir->fsDir);

		return Status::Success;
	}

	Status OpenFile(FS::Directory* dir, FS::File** file)
	{
		ASSERT(dir, "No dir");
		ASSERT(file, "Invalid pointer to file");

		if(dir->fsInfo != nullptr)
		{
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