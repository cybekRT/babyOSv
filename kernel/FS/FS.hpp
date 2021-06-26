#pragma once

#include"Block/Block.hpp"
#include"Container/LinkedList.h"

namespace FS
{
	enum class Status
	{
		Success = 0,
		EOF = 1,

		Undefined = 255
	};

	struct DirEntry
	{
		bool isValid : 1;
		bool isDirectory : 1;
		bool isSymlink : 1;
		bool isHidden : 1;

		u32 size;
		u8 name[256];
	};

	struct Directory;
	struct File;

	struct FSInfo
	{
		u32 (*Name)(u8* buffer);
		Status (*Probe)(Block::BlockPartition* part);

		Status (*Alloc)(Block::BlockPartition* part, void** fs);
		Status (*Dealloc)(Block::BlockPartition* part, void** fs);

		Status (*OpenRoot)(void* fs, Directory** dir);
		Status (*OpenDirectory)(void* fs, Directory* src, Directory** dir);
		Status (*CloseDirectory)(void* fs, Directory** dir);
		Status (*RewindDirectory)(void* fs, Directory* dir);

		Status (*ReadDirectory)(void* fs, Directory* dir, DirEntry* entry);
		Status (*ChangeDirectory)(void* fs, Directory* dir);

		Status (*OpenFile)(void* fs, Directory* dir, File** file);
		Status (*CloseFile)(void* fs, File** file);

		Status (*ReadFile)(void* fs, File* file, u8* buffer, u32 bufferSize, u32* readCount);
	};

	extern LinkedList<FSInfo*> filesystems;

	bool Init();

	void Register(FSInfo* info);
	void Unregister(FSInfo* info);
}
