#pragma once

#include"Block.hpp"

namespace FS
{
	enum class FileType
	{
		Normal,
		Directory
	};

	struct DirEntry
	{
		FileType type;
		u32 nameLength;
		u8 name[];
	};

	struct Directory;
	struct File;

	struct FSInfo
	{
		u32 (*Name)(u8* buffer);
		Status (*Probe)(Block::BlockInfo* info);

		Status (*Alloc)(Block::BlockInfo* info, void** fs);
		Status (*Dealloc)(Block::BlockInfo* info, void** fs);

		//Status OpenRoot(Directory** dir);
		Status (*OpenDirectory)(void* fs, u8* path, Directory** dir);
		Status (*CloseDirectory)(void* fs, Directory* dir);

		Status (*ReadDirectory)(void* fs, Directory* dir, DirEntry** entry);
		Status (*ChangeDirectory)(void* fs, Directory* dir, DirEntry* entry);
	};

	extern LinkedList<FSInfo*> filesystems;

	bool Init();

	void Register(FSInfo* info);
	void Unregister(FSInfo* info);
}
