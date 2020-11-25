#include"FS.hpp"
#include"Memory.h"

using FS::File;
using FS::Directory;
using FS::DirEntry;

namespace FS
{
	struct File
	{

	};

	struct Directory
	{
		u32 firstCluster;
		u32 currentCluster;
		u32 dataOffset;
		u8 buffer[512];
	};
}

namespace FS_FAT12
{
	struct BPB
	{
		u8 _unused1[3];
		u8 oem[8];
		u16 bytesPerSector;
		u8 sectorsPerCluster;
		u16 reservedSectors;
		u8 fatsCount;
		u16 rootEntriesCount;
		u16 totalSectors;
		u8 mediaType;
		u16 sectorsPerFat;
		u16 sectorsPerTrack;
		u16 headsCount;
		u32 hiddenSectors;
		u32 totalSectorsHigh;
		// EBR
		u8 driveNumber;
		u8 _unused2;
		u8 signature;
		u8 volumeId[4];
		u8 label[11];
		u8 systemId[8];
		u8 bootCode[448];
		u8 bootSignature[2];
	} __attribute__((packed));

	/*
	struc FAT12_DirectoryEntry
        ;.name                  resb 8+3
        .name                   resb 8
        .ext                    resb 3
        .attributes             resb 1
        .reserved               resb 2
        .createTime             resb 2
        .createDate             resb 2
        .accessDate             resb 2
        .clusterHigh            resb 2
        .modificationTime       resb 2
        .modificationDate       resb 2
        .cluster                resb 2
        .size                   resb 4
endstruc
*/

	struct FAT12_DirEntry
	{
		u8 name[8];
		u8 ext[3];
		u8 attributes;
		u16 reserved;
		u16 createTime;
	} __attribute__((packed));

	struct Info
	{
		Block::BlockInfo* dev;
		BPB* bpb;
		u32 firstRootSector;
		u32 firstDataSector;
		u8* fat;
	};

	u32 Name(u8* buffer)
	{
		char name[] = "FAT12";
		memcpy(buffer, name, sizeof(name));
		return sizeof(name);
	}

	Status Probe(Block::BlockInfo* info)
	{
		u8 buffer[512];

		Print("Probing FAT12...\n");
		if(info->Read(info->dev, 0, buffer))
			return Status::Fail;

		if(buffer[0] == 0xEB && buffer[2] == 0x90)
			return Status::Success;

		return Status::Fail;
	}

	Status Alloc(Block::BlockInfo* dev, void** fs)
	{
		dev->Lock(dev);

		BPB* bpb = (BPB*)Memory::Alloc(sizeof(BPB));
		dev->Read(dev, 0, (u8*)bpb);

		Print("Sectors per FAT: %d\n", bpb->sectorsPerFat);

		u32 fatSize = bpb->sectorsPerFat * dev->BlockSize(dev);
		Print("Block size: %u\n", dev->BlockSize(dev));
		Print("Block size: %d %d %d %d\n", 123, 256, 512, 1024);

		Print("Reading FAT... ");
		u8* fat = (u8*)Memory::Alloc(fatSize);
		for(unsigned a = 0; a < bpb->sectorsPerFat; a++)
		{
			dev->Read(dev, bpb->reservedSectors + bpb->hiddenSectors + a, fat + (a * 512));
		}
		Print("Done!\n");

		Info* info = (Info*)Memory::Alloc(sizeof(Info));
		info->dev = dev;
		info->bpb = bpb;
		info->fat = fat;

		info->firstRootSector = info->bpb->reservedSectors 
					+ info->bpb->hiddenSectors 
					+ info->bpb->sectorsPerFat * info->bpb->fatsCount;
		info->firstDataSector = info->firstRootSector + info->bpb->rootEntriesCount * 32 / 512;

		*fs = (void*)info;

		dev->Unlock(dev);
		return Status::Success;
	}

	Status Dealloc(Block::BlockInfo* info, void** fs)
	{

	}

	Status OpenRoot(void* fs, Directory** dir)
	{
		Info* info = (Info*)fs;
		*dir = (Directory*)Memory::Alloc(sizeof(Directory));

		(*dir)->firstCluster = 0;
		(*dir)->currentCluster = (*dir)->firstCluster;
		(*dir)->dataOffset = 0;

		return Status::Success;
	}

	Status OpenDirectory(void* fs, u8* path, Directory** dir)
	{
		Info* info = (Info*)fs;
	}

	Status CloseDirectory(void* fs, Directory* dir, DirEntry** entry)
	{
		Info* info = (Info*)fs;

		if(entry != nullptr)
		{
			Memory::Free(entry);
		}

		Memory::Free(dir);
	}

	Status ReadDirectory(void* fs, Directory* dir, DirEntry** entry)
	{
		Info* info = (Info*)fs;

		if(dir->dataOffset == 0 || dir->dataOffset == 512)
		{
			if(dir->dataOffset == 512)
			{
				dir->currentCluster++;
				dir->dataOffset = 0;
			}

			u32 firstSector = (dir->firstCluster == 0) ? info->firstRootSector : info->firstDataSector - 2;

			Print("Ptr: %p, %p, %p\n", info, info->dev, info->dev->Read);
			info->dev->Read(info->dev, firstSector + dir->currentCluster, dir->buffer);
			if(dir->currentCluster == 0)
				dir->dataOffset += 32;
		}

		FAT12_DirEntry* fatEntry = (FAT12_DirEntry*)(dir->buffer + dir->dataOffset);

		*entry = (DirEntry*)Memory::Alloc(sizeof(DirEntry) + 8+3+1);
		(*entry)->type = (fatEntry->attributes & (1 << 4)) ? FS::FileType::Directory : FS::FileType::Normal;
		for(unsigned a = 0; a < 8 + 3; a++)
			(*entry)->name[a] = fatEntry->name[a];
		(*entry)->name[8 + 3] = 0;
		(*entry)->nameLength = 8 + 3;

		dir->dataOffset += 32;

		return Status::Success;
	}

	Status ChangeDirectory(void* fs, Directory* dir, DirEntry* entry)
	{

	}

	FS::FSInfo info = 
	{
		.Name = Name,
		.Probe = Probe,

		.Alloc = Alloc,
		.Dealloc = Dealloc,

		.OpenRoot = OpenRoot,
		.OpenDirectory = OpenDirectory,
		.CloseDirectory = CloseDirectory,
		
		.ReadDirectory = ReadDirectory,
		.ChangeDirectory = ChangeDirectory,
	};

	bool Init()
	{
		FS::Register(&info);

		return true;
	}
}