#pragma once

#include"FS/FS.hpp"
#include"Path.hpp"

namespace VFS
{
	Status Init();

	//Status Mount(Block::BlockPartition* part);
	Status Mount(char* partName, char* mountPoint, char* fsType = nullptr);

	Status OpenRoot(FS::Directory** dir);
	Status OpenDirectory(FS::Directory* src, FS::Directory** dir);
	Status CloseDirectory(FS::Directory** dir);
	Status RewindDirectory(FS::Directory* dir);

	Status ReadDirectory(FS::Directory* dir, FS::DirEntry* entry);
	Status ChangeDirectory(FS::Directory* dir, u8* name);
	//Status GetPath(FS::Directory* dir, u8* buffer);
	Status GetPath(FS::Directory* dir, Path& path);

	Status OpenFile(FS::Directory* dir, u8* name, FS::File** file);
	Status CloseFile(FS::File** file);

	Status ReadFile(FS::File* file, u8* buffer, u32 bufferSize, u32* readCount);
}