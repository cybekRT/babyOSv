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

	struct Info
	{
		Block::BlockInfo* dev;
		BPB* bpb;
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

		Print("Proving FAT12...\n");
		if(info->Read(info->dev, 0, buffer))
			return Status::Fail;

		if(buffer[0] == 0xEB && buffer[2] == 0x90)
			return Status::Success;

		return Status::Fail;
	}

	Status Alloc(Block::BlockInfo* dev, void** fs)
	{
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
		*fs = (void*)info;

		return Status::Success;
	}

	Status Dealloc(Block::BlockInfo* info, void** fs)
	{

	}

	Status OpenDirectory(void* fs, u8* path, Directory** dir)
	{

	}

	Status CloseDirectory(void* fs, Directory* dir)
	{

	}

	Status ReadDirectory(void* fs, Directory* dir, DirEntry** entry)
	{

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