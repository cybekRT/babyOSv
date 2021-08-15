#include"BlkFS.hpp"
#include"FS.hpp"
#include"Memory.h"

using FS::File;
using FS::Directory;
using FS::DirEntry;

namespace FS
{
	enum class Type
	{
		Root = -1,
		Devices = 0,
		Partitions = 1,

		Count
	};

	struct File
	{
		Type type;
		u32 index;
		u32 offset;

		u32 dataLen;
		u8 data[512];
	};

	struct Directory
	{
		Type type;
		s32 index;
	};
}

namespace FS::BlkFS
{
	u32 Name(u8* buffer)
	{
		return strcpy("BlkFS", (char*)buffer);
	}

	FS::Status Probe(Block::BlockPartition* part)
	{
		return FS::Status::Success;
	}

	FS::Status Alloc(Block::BlockPartition* part, void** fs)
	{
		*fs = nullptr;

		return FS::Status::Success;
	}

	FS::Status Dealloc(Block::BlockPartition* part, void** fs)
	{
		return FS::Status::Success;
	}

	/*
	 *
	 *  Directory
	 *
	 */

	FS::Status OpenRoot(void* fs, Directory** dir)
	{
		*dir = (Directory*)Memory::Alloc(sizeof(Directory));

		(*dir)->type = Type::Root;
		(*dir)->index = -1;

		return FS::Status::Success;
	}

	FS::Status OpenDirectory(void* fs, Directory* src, Directory** dir)
	{
		return Status::Undefined;
	}

	FS::Status CloseDirectory(void* fs, Directory** dir)
	{
		Memory::Free(*dir);
		*dir = nullptr;

		return FS::Status::Success;
	}

	FS::Status RewindDirectory(void* fs, Directory* dir)
	{
		dir->index = -1;

		return FS::Status::Success;
	}

	/*
	 * Root
	 */

	FS::Status ReadDirectory_Root(void* fs, Directory* dir, DirEntry* entry)
	{
		const char* dirs[] = { "dev", "part" };

		if((u32)dir->index >= sizeof(dirs) / sizeof(dirs[0]))
			return Status::EOF;

		entry->isDirectory = true;
		strcpy(dirs[dir->index], (char*)entry->name);

		return Status::Success;
	}

	/*
	 * Devices
	 */

	FS::Status ReadDirectory_Devices(void* fs, Directory* dir, DirEntry* entry)
	{
		auto devices = Block::GetDevices();
		if((u32)dir->index >= devices.Size())
			return Status::EOF;

		auto dev = devices[dir->index];
		strcpy((char*)dev->name, (char*)entry->name);

		return Status::Success;
	}

	/*
	 * Partitions
	 */

	FS::Status ReadDirectory_Partitions(void* fs, Directory* dir, DirEntry* entry)
	{
		auto partitions = Block::GetPartitions();
		if((u32)dir->index >= partitions.Size())
			return Status::EOF;

		auto part = partitions[dir->index];
		strcpy((char*)part->name, (char*)entry->name);

		return Status::Success;
	}

	FS::Status ReadDirectory(void* fs, Directory* dir, DirEntry* entry)
	{
		dir->index++;

		entry->isDirectory = entry->isHidden = entry->isSymlink = entry->isValid = false;
		entry->size = 0;

		Status status;
		switch(dir->type)
		{
			case Type::Devices:
				status = ReadDirectory_Devices(fs, dir, entry);
				break;
			case Type::Partitions:
				status = ReadDirectory_Partitions(fs, dir, entry);
				break;
			default:
				status = ReadDirectory_Root(fs, dir, entry);
				break;
		}

		entry->isValid = (status == Status::Success);
		return status;
	}

	FS::Status ChangeDirectory(void* fs, Directory* dir)
	{
		if((u32)dir->index < 0 || (u32)dir->index >= (u32)Type::Count)
			return Status::Undefined;

		if(dir->type == Type::Root)
			dir->type = (Type)dir->index;
		else
			return Status::Undefined;

		return Status::Success;
	}

	/*
	 *
	 *  File
	 *
	 */

	FS::Status OpenFile_Devices(void* fs, Directory* dir, File** file)
	{
		Terminal::SetBuffer((*file)->data);

		auto devices = Block::GetDevices();
		if((*file)->index >= devices.Size())
			return Status::Undefined;

		auto dev = devices[(*file)->index];
		(*file)->dataLen = Print("Device: %s\nBlocks: %u\n", dev->name, dev->drv->Size(dev->drvPriv));

		Terminal::SetBuffer(nullptr);
		return Status::Success;
	}

	FS::Status OpenFile(void* fs, Directory* dir, File** file)
	{
		if(dir->type == Type::Root)
			return Status::Undefined;

		(*file) = (File*)Memory::Alloc(sizeof(File));

		(*file)->type = dir->type;
		(*file)->index = dir->index;
		(*file)->offset = 0;

		Status status;
		switch(dir->type)
		{
			case Type::Devices:
			{
				status = OpenFile_Devices(fs, dir, file);
				break;
			}
			default:
			{
				status = Status::Undefined;
				break;
			}
		}

		return status;
	}

	FS::Status CloseFile(void* fs, File** file)
	{
		Memory::Free(*file);
		(*file) = nullptr;

		return Status::Success;
	}

	FS::Status ReadFile(void* fs, File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{
		if(file->offset >= file->dataLen)
			return Status::EOF;

		u32 len = (file->dataLen - file->offset < bufferSize) ? file->dataLen - file->offset : bufferSize;
		memcpy(buffer, file->data + file->offset, len);
		(*readCount) = len;
		
		file->offset += len;
		
		return Status::Success;
	}

	/*
	 *
	 *  FSInfo
	 *
	 */

	FS::FSInfo info = 
	{
		.Name = Name,
		.Probe = Probe,

		.Alloc = Alloc,
		.Dealloc = Dealloc,

		.OpenRoot = OpenRoot,
		.OpenDirectory = OpenDirectory,
		.CloseDirectory = CloseDirectory,
		.RewindDirectory = RewindDirectory,
		
		.ReadDirectory = ReadDirectory,
		.ChangeDirectory = ChangeDirectory,

		.OpenFile = OpenFile,
		.CloseFile = CloseFile,

		.ReadFile = ReadFile,
	};

	bool Init()
	{
		FS::Register(&info);

		return true;
	}
}