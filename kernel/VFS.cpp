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
		if((*dir)->fsDir != nullptr)
		{
			(*dir)->fsInfo->CloseDirectory((*dir)->fsPriv, &(*dir)->fsDir);
		}

		Memory::Free((void*)*dir);
		(*dir) = nullptr;
	}

	Status RewindDirectory(FS::Directory* dir)
	{
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
		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->ReadDirectory(dir->fsPriv, dir->fsDir, entry);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		dir->index++;
		auto ptr = Block::devices.data;

		unsigned a = 0;
		while(ptr)
		{
			if(a == dir->index)
			{
				entry->isValid = 1;
				entry->isDirectory = 1;
				entry->isHidden = 0;
				entry->isSymlink = 0;

				strcpy((char*)ptr->value->name, (char*)entry->name);

				return Status::Success;
			}
			else if(a >= dir->index)
				break;

			ptr = ptr->next;
		}

		return Status::Fail;
	}

	Status ChangeDirectory(FS::Directory* dir)
	{
		if(dir->fsInfo != nullptr)
		{
			auto fsResult = dir->fsInfo->ChangeDirectory(dir->fsPriv, dir->fsDir);
			Print("Result %x: %s:%d\n", fsResult, __FILE__, __LINE__);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		Block::BlockDevice* bd = nullptr;
		auto ptr = Block::devices.data;

		unsigned a = 0;
		while(ptr)
		{
			if(a == dir->index)
			{
				bd = ptr->value;
				break;
			}

			ptr = ptr->next;
		}

		if(!ptr)
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
		if(dir->fsInfo != nullptr)
		{
			(*file) = (FS::File*)Memory::Malloc(sizeof(FS::File));
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
		(*file)->fsInfo->CloseFile((*file)->fsPriv, &(*file)->fsFile);
		Memory::Mfree(*file);
		(*file) = nullptr;

		return Status::Success;
	}

	Status ReadFile(FS::File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{
		auto fsResult = file->fsInfo->ReadFile(file->fsPriv, file->fsFile, buffer, bufferSize, readCount);

		return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
	}
}