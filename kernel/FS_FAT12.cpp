#include"FS.hpp"
#include"Memory.h"

//using FS::Status;
using FS::File;
using FS::Directory;
using FS::DirEntry;
#include"Timer.h"
u8 tolower(u8 c)
{
	if(c >= 'A' && c <= 'Z')
	{
		return c - ('A' - 'a');
	}
	else
	{
		return c;
	}
}

u8 toupper(u8 c)
{
	if(c >= 'a' && c <= 'z')
	{
		return c - ('a' - 'A');
	}
	else
	{
		return c;
	}
}

namespace FS
{
	struct File
	{
		u32 firstCluster;
		u32 currentCluster;
		u32 totalDataOffset; // Total data offset, not position in buffer
		u32 size;
		u8 bufferValid;
		u8 buffer[512];
	};

	struct Directory
	{
		u32 firstCluster;
		u32 currentCluster;
		u32 dataOffset;
		u8 bufferValid;
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

	struct Time
	{
		u16	hour : 5;
		u16	minutes : 6;
		u16	seconds : 5;
	} __attribute__((packed));

	struct Date
	{
		u16	year : 7;
		u16	month : 4;
		u16	day : 5;
	} __attribute__((packed));

	enum class Attribute
	{
		ReadOnly	=	1,
		Hidden		=	2,
		System		=	4,
		VolumeLabel	=	8,
		Directory	=	16,
		Archive		=	32,
		Device		=	64,
		Reserved	=	128,
	};
	inline Attribute operator|(Attribute a, Attribute b) { return (Attribute)(unsigned(a) | unsigned(b) ); }

	struct FAT12_DirEntry
	{
		u8		name[8+3];
		u8		attributes;
		u8		_reserved;
		u8		_unused;
		Time	createTime;
		Date	createDate;
		Date	accessDate;
		u16		clusterHigh;
		Time	modificationTime;
		Date	modificationDate;
		u16		cluster;
		u32		size;
	} __attribute__((packed));

	/*struct FAT12_DirEntry
	{
		u8 name[8];
		u8 ext[3];
		u8 attributes;
		u16 reserved;
		u16 createTime;
	} __attribute__((packed));*/

	struct Info
	{
		Block::BlockInfo* dev;
		BPB* bpb;
		u32 firstRootSector;
		u32 firstDataSector;
		u8* fat;
	};

	/*
	 *
	 *  Helper functions
	 *
	 */

	u16 NextCluster(void* fs, u16 cluster)
	{
		Info* info = (Info*)fs;

		u32 fatOffset = cluster + (cluster >> 1);
		//Print("Fat offset: %x - %x\n", cluster, fatOffset);
		u16 nextCluster = *(u16*)(info->fat + fatOffset);

		if(cluster & 1)
			nextCluster = nextCluster >> 4;
		else
			nextCluster = nextCluster & 0xfff;

		if(nextCluster >= 0xFF8) // Last one...
			nextCluster = 0;

		return nextCluster;
	}

	/*
	 *
	 *  Probe
	 *
	 */

	u32 Name(u8* buffer)
	{
		char name[] = "FAT12";
		memcpy(buffer, name, sizeof(name));
		return sizeof(name);
	}

	FS::Status Probe(Block::BlockInfo* info)
	{
		u8 buffer[512];

		Print("Probing FAT12...\n");
		if(info->Read(info->dev, 0, buffer))
			return FS::Status::Undefined;

		if(buffer[0] == 0xEB && buffer[2] == 0x90)
			return FS::Status::Success;

		return FS::Status::Undefined;
	}

	FS::Status Alloc(Block::BlockInfo* dev, void** fs)
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
		return FS::Status::Success;
	}

	FS::Status Dealloc(Block::BlockInfo* info, void** fs)
	{

	}

	/*
	 *
	 *  Directory
	 *
	 */

	FS::Status OpenRoot(void* fs, Directory** dir)
	{
		Info* info = (Info*)fs;
		*dir = (Directory*)Memory::Alloc(sizeof(Directory));

		(*dir)->firstCluster = 0;
		(*dir)->currentCluster = (*dir)->firstCluster;
		(*dir)->dataOffset = 0;
		(*dir)->bufferValid = 0;

		return FS::Status::Success;
	}

	FS::Status OpenDirectory(void* fs, Directory* src, Directory** dir)
	{
		Info* info = (Info*)fs;
		*dir = (Directory*)Memory::Alloc(sizeof(Directory));

		(*dir)->firstCluster = src->firstCluster;
		(*dir)->currentCluster = src->currentCluster;
		(*dir)->dataOffset = src->dataOffset;
		(*dir)->bufferValid = src->bufferValid;
		memcpy((*dir)->buffer, src->buffer, sizeof(src->buffer));

		return FS::Status::Success;
	}

	FS::Status CloseDirectory(void* fs, Directory** dir)
	{
		Info* info = (Info*)fs;

		Memory::Free(*dir);
		*dir = nullptr;

		return FS::Status::Success;
	}

	FS::Status RewindDirectory(void* fs, Directory* dir)
	{
		dir->currentCluster = dir->firstCluster;
		dir->dataOffset = 0;
		dir->bufferValid = 0;

		return FS::Status::Success;
	}

	FS::Status ReadDirectory(void* fs, Directory* dir, DirEntry* entry)
	{
		Info* info = (Info*)fs;

		if(dir->bufferValid)
			dir->dataOffset += 32;

		if(dir->dataOffset == 0 || dir->dataOffset == 512)
		{
			if(dir->dataOffset == 512)
			{
				if(dir->firstCluster == 0)
					dir->currentCluster++;
				else
					dir->currentCluster = NextCluster(fs, dir->currentCluster);

				dir->dataOffset = 0;
			}

			if(dir->firstCluster == 0 && dir->currentCluster >= (info->bpb->rootEntriesCount * 32 / 512))
			{
				dir->bufferValid = 0;
				return FS::Status::EOF;
			}

			if(dir->firstCluster != 0 && dir->currentCluster == 0)
			{
				dir->bufferValid = 0;
				return FS::Status::EOF;
			}

			u32 firstSector = (dir->firstCluster == 0) ? info->firstRootSector : info->firstDataSector - 2;

			info->dev->Read(info->dev, firstSector + dir->currentCluster, dir->buffer);
			if(dir->currentCluster == 0)
				dir->dataOffset += 32;

			dir->bufferValid = 1;
		}

		FAT12_DirEntry* fatEntry = (FAT12_DirEntry*)(dir->buffer + dir->dataOffset);

		entry->isDirectory = fatEntry->attributes & (u8)Attribute::Directory;
		entry->isSymlink = 0;
		entry->isValid = (fatEntry->name[0] != 0 && fatEntry->name[0] != ' ');
		entry->size = fatEntry->size;

		u8* dst = entry->name;
		for(unsigned a = 0; a < 8; a++)
		{
			u8 c = fatEntry->name[a];
			if(c == ' ')
				break;

			(*dst++) = tolower(c);
		}

		if(fatEntry->name[8] != ' ')
		{
			(*dst++) = '.';
			for(unsigned a = 0; a < 3; a++)
			{
				u8 c = fatEntry->name[8 + a];
				if(c == ' ')
					break;

				(*dst++) = tolower(c);
			}
		}

		(*dst) = 0;

		return FS::Status::Success;
	}

	FS::Status ChangeDirectory(void* fs, Directory* dir)
	{
		Print("Changing directory...\n");
		FAT12_DirEntry* fatEntry = (FAT12_DirEntry*)(dir->buffer + dir->dataOffset);

		if(!(fatEntry->attributes & (u8)Attribute::Directory))
		{
			Print("Not a directory!!\n");
			return FS::Status::Undefined;
		}

		dir->firstCluster = dir->currentCluster = fatEntry->cluster;
		Print("Directory cluster: %x\n", dir->firstCluster);
		dir->dataOffset = 0;
		dir->bufferValid = 0;

		return FS::Status::Success;
	}

	/*
	 *
	 *  File
	 *
	 */

	FS::Status OpenFile(void* fs, Directory* dir, File** file)
	{
		Info* info = (Info*)fs;
		FAT12_DirEntry* fatEntry = (FAT12_DirEntry*)(dir->buffer + dir->dataOffset);

		(*file) = (File*)Memory::Alloc(sizeof(File));

		(*file)->firstCluster = fatEntry->cluster;
		(*file)->currentCluster = (*file)->firstCluster;

		(*file)->totalDataOffset = 0;
		(*file)->bufferValid = 0;
		(*file)->size = fatEntry->size;

		return FS::Status::Success;
	}

	FS::Status CloseFile(void* fs, File** file)
	{
		Info* info = (Info*)fs;

		Memory::Free(*file);
		(*file) = nullptr;

		return FS::Status::Success;
	}

	FS::Status ReadFile(void* fs, File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{
		static u32 ttttt = 0;
		Info* info = (Info*)fs;

		(*readCount) = 0;

		if(file->totalDataOffset >= file->size)
			return FS::Status::EOF;

		/*if(file->totalDataOffset > 0 && (file->totalDataOffset % sizeof(file->buffer)) == 0)
		{
			file->currentCluster = NextCluster(fs, file->currentCluster);
			file->bufferValid = 0;
		}*/

		//if(file->currentCluster == 0)
		//	return FS::Status::EOF;

		if(!file->bufferValid)
		{
			if(file->totalDataOffset > 0)
			{
				file->currentCluster = NextCluster(fs, file->currentCluster);
				//file->bufferValid = 0;

				if(file->currentCluster == 0)
					return FS::Status::EOF;
			}

			u32 sector = info->firstDataSector + file->currentCluster - 2;
			info->dev->Read(info->dev, sector, file->buffer);
			file->bufferValid = 1;
		}

		// If there's need to read more data than in internal buffer, divide to 2 separate calls
		u32 inBuffer = sizeof(file->buffer) - (file->totalDataOffset % sizeof(file->buffer));
		if(bufferSize > inBuffer)
		{
			Print("\n----- Subreading... (%u / %u) -----\n", inBuffer, bufferSize);

			//Timer::Delay(50);
			u32 subread;
			auto s1 = ReadFile(fs, file, buffer, inBuffer, &subread);
			(*readCount) += subread;

			if(s1 != FS::Status::Success)
			{
				Print("S1 = %x\n", s1);
				return s1;
			}

			for(unsigned a = 0; a < bufferSize; )
			{
				u32 curSize = (bufferSize - (*readCount) > sizeof(file->buffer) ? sizeof(file->buffer) : bufferSize - (*readCount));
				Print(".");
				auto s2 = ReadFile(fs, file, buffer + (*readCount), curSize, &subread);
				Print("Read: %u\n", subread);
				a += subread;
				(*readCount) += subread;

				if(s2 != FS::Status::Success)
				{
					Print("Fail!\n");
					return s2;
				}
			}

			Print("OK!\n");
			return FS::Status::Success;

			auto s2 = ReadFile(fs, file, buffer + subread, bufferSize - subread, &subread);
			(*readCount) += subread;

			if(s2 == FS::Status::EOF)
			{
				Print("S2 EOF = %x, %x\n", s1, s2);
				return s1;
			}

			//Print("S2 = %x\n", s1);
			return s2;
		}
		else
		{
			Print("no subread: %u %u\n", inBuffer, bufferSize);
		}
		

		// Otherwise, read from internal buffer

		u32 offset = (file->totalDataOffset % sizeof(file->buffer));
		Print("Offset: %u, %u\n", offset, bufferSize);
		for(unsigned a = 0; a < bufferSize; a++)
		{
			if(file->totalDataOffset >= file->size)
			{
				//return FS::Status::EOF;
				break;
			}

			//Print("%u, %u, %u\n", a, offset, offset+a);
			buffer[a] = file->buffer[offset + a];

			file->totalDataOffset++;
			(*readCount)++;
		}

		if(file->totalDataOffset % sizeof(file->buffer) == 0)
			file->bufferValid = 0;

		ttttt += *readCount;
		Print("Total: %u\n", ttttt);

		if((*readCount) == 0)
		{
			Print("EOF\n");
			return FS::Status::EOF;
		}

		return FS::Status::Success;
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