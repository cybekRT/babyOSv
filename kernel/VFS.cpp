#include"FS.hpp"

namespace FS
{
	struct File
	{
		
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
		Memory::Free((void*)*dir);
		(*dir) = nullptr;
	}

	Status RewindDirectory(FS::Directory* dir)
	{
		if(dir->fsInfo == nullptr)
		{
			dir->index = -1;
		}
		else
		{
			auto fsResult = dir->fsInfo->RewindDirectory(dir->fsPriv, dir->fsDir);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}
	}

	Status ReadDirectory(FS::Directory* dir, FS::DirEntry* entry)
	{
		if(dir->fsInfo == nullptr)
		{
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

					ptr->value->Name(ptr->value, entry->name);

					return Status::Success;
				}
				else if(a >= dir->index)
					break;

				ptr = ptr->next;
			}
		}
		else
		{
			auto fsResult = dir->fsInfo->ReadDirectory(dir->fsPriv, dir->fsDir, entry);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}

		return Status::Fail;
	}

	Status ChangeDirectory(FS::Directory* dir)
	{
		if(dir->fsInfo == nullptr)
		{
			Block::BlockInfo* blockInfo = nullptr;
			auto ptr = Block::devices.data;

			unsigned a = 0;
			while(ptr)
			{
				if(a == dir->index)
				{
					blockInfo = ptr->value;
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
				if(fsItr->value->Probe(blockInfo) == FS::Status::Success)
					break;

				fsItr = fsItr->next;
			}

			if(!fsItr)
			{
				Print("Fail: %s:%d\n", __FILE__, __LINE__);
				return Status::Fail;
			}

			dir->fsInfo = fsItr->value;
			dir->fsInfo->Alloc(blockInfo, &dir->fsPriv);
			dir->fsInfo->OpenRoot(dir->fsPriv, &dir->fsDir);

			return Status::Success;
		}
		else
		{
			auto fsResult = dir->fsInfo->ChangeDirectory(dir->fsPriv, dir->fsDir);
			Print("Result %x: %s:%d\n", fsResult, __FILE__, __LINE__);
			return (fsResult == FS::Status::Success ? Status::Success : Status::Fail);
		}
	}

	Status OpenFile(FS::Directory* dir, FS::File** file)
	{

	}

	Status CloseFile(FS::File** file)
	{

	}

	Status ReadFile(FS::File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{

	}
}