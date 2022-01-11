#pragma once

#include"Block/Block.hpp"
#include"Container/LinkedList.hpp"
#include"Status.hpp"

namespace FS
{
	constexpr u32 MaxFilenameLength = 256;

	struct DirEntry
	{
		bool isValid : 1;
		bool isDirectory : 1;
		bool isSymlink : 1;
		bool isHidden : 1;

		u32 size;
		u8 name[MaxFilenameLength];
	};

	struct Directory;
	struct File;

	struct FSInfo
	{
		char* name;

		Status (*Probe)(Block::BlockPartition* part);
		Status (*Format)(Block::BlockPartition* part, void* params);

		Status (*Mount)(Block::BlockPartition* part, void** fs);
		Status (*Unmount)(Block::BlockPartition* part, void** fs);

		Status (*LabelGet)(void* fs, char* buffer, u32* bufferSize);
		Status (*LabelSet)(void* fs, char* buffer);

		Status (*DirectoryOpenRoot)(void* fs, Directory** dir);
		Status (*DirectoryClose)(void* fs, Directory** dir);

		Status (*DirectoryRead)(void* fs, Directory* dir, DirEntry* entry);
		Status (*DirectoryRewind)(void* fs, Directory* dir);
		Status (*DirectoryFollow)(void* fs, Directory* dir);
		Status (*DirectoryCreate)(void* fs, Directory* dir, char* name);
		Status (*DirectoryRemove)(void* fs, Directory* dir);

		Status (*FileCreate)(void* fs, Directory* dir, char* name);
		Status (*FileDelete)(void* fs, Directory* dir);
		Status (*FileOpen)(void* fs, Directory* dir, File** file);
		Status (*FileClose)(void* fs, File** file);

		Status (*FileRead)(void* fs, File* file, u8* buffer, u32 bufferSize, u32* readCount);
		Status (*FileWrite)(void* fs, File* file, u8* buffer, u32 bufferSize, u32* writeCount);

		Status (*FileSetPointer)(void* fs, File* file, u32 offset);
		Status (*FileGetPointer)(void* fs, File* file, u32* offset);
	};

	extern Container::LinkedList<FSInfo*> filesystems;

	bool Init();

	void Register(FSInfo* info);
	void Unregister(FSInfo* info);
}
