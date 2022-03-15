#pragma once

#include"FS/FS.hpp"
#include"Path.hpp"

namespace VFS
{
	Status Init();

	Status Mount(char* partName, char* mountPoint, char* fsType = nullptr);
	Status Unmount(char* mountPoint);
	Status Flush();

	Status DirectoryOpenRoot(FS::Directory** dir);
	Status DirectoryClose(FS::Directory** dir);
	Status DirectoryRead(FS::Directory* dir, FS::DirEntry* entry);
	Status DirectoryRewind(FS::Directory* dir);
	Status DirectoryChange(FS::Directory* dir, char* name);
	Status DirectoryCreate(FS::Directory* dir, char* name);
	Status DirectoryRemove(FS::Directory* dir, char* name);
	Status DirectoryGetPath(FS::Directory* dir, Path& path);

	Status FileCreate(FS::Directory* dir, char* name);
	Status FileDelete(FS::Directory* dir, char* name);
	Status FileOpen(FS::Directory* dir, char* name, FS::File** file);
	Status FileClose(FS::File** file);
	Status FileRead(FS::File* file, u8* buffer, u32 bufferSize, u32* readCount);
	Status FileWrite(FS::File* file, u8* buffer, u32 bufferSize, u32* writeCount);
	Status FileSetPointer(FS::File* file, u32 offset);
	Status FileGetPointer(FS::File* file, u32* offset);
}