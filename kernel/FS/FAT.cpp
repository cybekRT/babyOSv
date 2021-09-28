#include"FS.hpp"
#include"Memory.h"

using FS::File;
using FS::Directory;
using FS::DirEntry;

u8 tolower(u8 c);
u8 toupper(u8 c);

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
		//u8 buffer[512 * 4];
		u32 bufferSize;
		u8 buffer[];
	};

	struct Directory
	{
		u32 firstCluster;
		u32 currentCluster;
		u32 dataOffset;
		u8 bufferValid;
		u32 bufferSize;
		//u8 buffer[512 * 4];
		u8 buffer[];
	};
}

namespace FAT
{
	enum class Type
	{
		Invalid = 0,
		FAT12 = 12,
		FAT16 = 16,
		FAT32 = 32
	};

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

	struct FAT16_DirEntry
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

	struct Info
	{
		Type type;
		Block::BlockPartition* part;
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

	u32 NextCluster(void* fs, u16 cluster)
	{
		Info* info = (Info*)fs;
		u32 nextCluster;

		if(info->type == Type::FAT12)
		{
			u32 fatOffset = cluster + (cluster >> 1);
			nextCluster = *(u16*)(info->fat + fatOffset);

			if(cluster & 1)
				nextCluster = nextCluster >> 4;
			else
				nextCluster = nextCluster & 0xfff;

			if(nextCluster >= 0xFF8) // Last one...
				nextCluster = 0;
		}
		else if(info->type == Type::FAT16)
		{
			nextCluster = *((u16*)info->fat + cluster);

			if(nextCluster >= 0xFFF8) // Last one...
				nextCluster = 0;
		}
		else
		{
			nextCluster = 0;
		}

		return nextCluster;
	}

	/*
	 *
	 *  Probe
	 *
	 */

	u32 Name(u8* buffer)
	{
		char name[] = "FAT";
		memcpy(buffer, name, sizeof(name));
		return sizeof(name);
	}

	FS::Status Probe(Block::BlockPartition* part)
	{
		u8 buffer[512];

		Print("Probing FAT16...\n");
		//if(bd->drv->Read(bd->drvPriv, 0, buffer))
		if(part->Read(0, buffer))
			return FS::Status::Undefined;

		if(buffer[0] == 0xEB && buffer[2] == 0x90)
			return FS::Status::Success;

		return FS::Status::Undefined;
	}

	FS::Status Alloc(Block::BlockPartition* part, void** fs)
	{
		part->device->drv->Lock(part->device->drvPriv);
		//bd->drv->Lock(bd->drvPriv);

		BPB* bpb = (BPB*)Memory::Alloc(sizeof(BPB));
		//bd->drv->Read(bd->drvPriv, 0, (u8*)bpb);
		part->Read(0, (u8*)bpb);

		Print("Sectors per FAT: %d\n", bpb->sectorsPerFat);

		u32 fatSize = bpb->sectorsPerFat * part->device->drv->BlockSize(part->device->drvPriv);

		Print("Reading FAT... ");
		u8* fat = (u8*)Memory::Alloc(fatSize);
		for(unsigned a = 0; a < bpb->sectorsPerFat; a++)
		{
			//bd->drv->Read(bd->drvPriv, bpb->reservedSectors + bpb->hiddenSectors + a, fat + (a * 512));
			part->Read(bpb->reservedSectors /*+ bpb->hiddenSectors*/ + a, fat + (a * 512));
		}
		Print("Done!\n");

		u32 totalSectors = (bpb->totalSectors > 0) ? bpb->totalSectors : bpb->totalSectorsHigh;

		Info* info = (Info*)Memory::Alloc(sizeof(Info));

		if(totalSectors < 4085)
			info->type = Type::FAT12;
		else if(totalSectors < 65525)
			info->type = Type::FAT16;
		else if(totalSectors < 268435445)
			info->type = Type::FAT32;
		else
			info->type = Type::Invalid;

		Print("Type: FAT%d\n", info->type);

		info->part = part;
		info->bpb = bpb;
		info->fat = fat;

		info->firstRootSector = info->bpb->reservedSectors 
					+ info->bpb->sectorsPerFat * info->bpb->fatsCount;
		info->firstDataSector = info->firstRootSector + info->bpb->rootEntriesCount * 32 / 512;
		Print("Root entries: %d\n", info->bpb->rootEntriesCount);

		Print("Reserved: %d, Hidden: %d, SectorePerFat: %d, FatsCount: %d\n", 
				info->bpb->reservedSectors, 
				info->bpb->hiddenSectors, 
				info->bpb->sectorsPerFat, 
				info->bpb->fatsCount);
		Print("Root sector: %d, root entries: %d\n", info->firstRootSector, info->bpb->rootEntriesCount);
		Print("Data sector: %d\n", info->firstDataSector);
		Print("Sectors per cluster: %d\n", info->bpb->sectorsPerCluster);

		*fs = (void*)info;

		part->device->drv->Lock(part->device->drvPriv);
		//bd->drv->Unlock(bd->drvPriv);
		return FS::Status::Success;
	}

	FS::Status Dealloc(Block::BlockPartition* part, void** fs)
	{
		if(!fs)
			return FS::Status::Success;

		Info* info = (Info*)(*fs);
		Memory::Free(info->bpb);
		Memory::Free(info->fat);
		Memory::Free(info);

		(*fs) = nullptr;
		return FS::Status::Success;
	}

	/*
	 *
	 *  Directory
	 *
	 */

	FS::Status OpenRoot(void* fs, Directory** dir)
	{
		Info* info = (Info*)fs;
		u32 bufferSize = info->part->device->drv->BlockSize(info->part->device->drvPriv) * info->bpb->sectorsPerCluster;
		*dir = (Directory*)Memory::Alloc(sizeof(Directory) + bufferSize);

		(*dir)->firstCluster = 0;
		(*dir)->currentCluster = (*dir)->firstCluster;
		(*dir)->dataOffset = 0;
		(*dir)->bufferValid = 0;
		(*dir)->bufferSize = bufferSize;

		return FS::Status::Success;
	}

	FS::Status OpenDirectory(void* fs, Directory* src, Directory** dir)
	{
		Info* info = (Info*)fs;
		*dir = (Directory*)Memory::Alloc(sizeof(Directory) + src->bufferSize);

		(*dir)->firstCluster = src->firstCluster;
		(*dir)->currentCluster = src->currentCluster;
		(*dir)->dataOffset = src->dataOffset;
		(*dir)->bufferValid = src->bufferValid;
		(*dir)->bufferSize = src->bufferSize;
		memcpy((*dir)->buffer, src->buffer, src->bufferSize);

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
		auto part = info->part;

		if(dir->bufferValid)
			dir->dataOffset += 32;

		if(dir->dataOffset == 0 || dir->dataOffset == 512 * info->bpb->sectorsPerCluster)
		{
			if(dir->dataOffset == 512 * info->bpb->sectorsPerCluster)
			{
				if(dir->firstCluster == 0)
					dir->currentCluster++;
				else
					dir->currentCluster = NextCluster(fs, dir->currentCluster);

				dir->dataOffset = 0;
			}

			if(dir->firstCluster == 0 && dir->currentCluster >= (info->bpb->rootEntriesCount * 32 / (512 * info->bpb->sectorsPerCluster)))
			{
				dir->bufferValid = 0;
				return FS::Status::EOF;
			}

			if(dir->firstCluster != 0 && dir->currentCluster == 0)
			{
				dir->bufferValid = 0;
				return FS::Status::EOF;
			}

			u32 firstSector = (dir->firstCluster == 0) 
					? (info->firstRootSector + dir->currentCluster * info->bpb->sectorsPerCluster) 
					: (info->firstDataSector + (dir->currentCluster - 2) * info->bpb->sectorsPerCluster);

			for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
			{
				part->Read(firstSector + a, dir->buffer + 512 * a);
			}

			dir->bufferValid = 1;
		}

		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

		entry->isDirectory = fatEntry->attributes & (u8)Attribute::Directory;
		entry->isSymlink = 0;
		entry->isValid = (fatEntry->name[0] != 0 && fatEntry->name[0] != ' ' && fatEntry->name[0] != 0xE5); // < deleted
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
		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

		if(!(fatEntry->attributes & (u8)Attribute::Directory))
		{
			Print("Not a directory!!\n");
			return FS::Status::Undefined;
		}

		dir->firstCluster = dir->currentCluster = fatEntry->cluster;
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
		u32 bufferSize = info->part->device->drv->BlockSize(info->part->device->drvPriv) * info->bpb->sectorsPerCluster;
		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

		(*file) = (File*)Memory::Alloc(sizeof(File) + bufferSize);

		(*file)->firstCluster = (fatEntry->clusterHigh << 16) | fatEntry->cluster;
		(*file)->currentCluster = (*file)->firstCluster;

		(*file)->totalDataOffset = 0;
		(*file)->bufferValid = 0;
		(*file)->bufferSize = bufferSize;
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
		Info* info = (Info*)fs;
		auto part = info->part;

		(*readCount) = 0;

		if(file->totalDataOffset >= file->size)
			return FS::Status::EOF;

		if(!file->bufferValid)
		{
			if(file->totalDataOffset > 0)
			{
				file->currentCluster = NextCluster(fs, file->currentCluster);

				if(file->currentCluster == 0)
					return FS::Status::EOF;
			}

			u32 sector = info->firstDataSector + (file->currentCluster - 2) * info->bpb->sectorsPerCluster;
			Print("Reading sector: %d (%d)\n", sector, info->bpb->sectorsPerCluster);
			part->Read(sector, file->buffer);
			file->bufferValid = 1;
		}

		// If there's need to read more data than in internal buffer, divide to 2 separate calls
		u32 inBuffer = file->bufferSize - (file->totalDataOffset % file->bufferSize);
		if(bufferSize > inBuffer)
		{
			FS::Status status;
			u32 subread;

			part->device->drv->Lock(part->device->drvPriv);
			for(unsigned a = 0; a < bufferSize; )
			{
				u32 curSize = (bufferSize - (*readCount) > file->bufferSize ? file->bufferSize : bufferSize - (*readCount));

				status = ReadFile(fs, file, buffer + (*readCount), curSize, &subread);
				Print("Read: %u\n", subread);
				a += subread;
				(*readCount) += subread;

				if(status != FS::Status::Success)
				{
					break;
				}
			}

			part->device->drv->Unlock(part->device->drvPriv);

			return status;
		}		

		// Otherwise, read from internal buffer
		u32 offset = (file->totalDataOffset % file->bufferSize);
		for(unsigned a = 0; a < bufferSize; a++)
		{
			if(file->totalDataOffset >= file->size)
				break;

			buffer[a] = file->buffer[offset + a];

			file->totalDataOffset++;
			(*readCount)++;
		}

		if(file->totalDataOffset % file->bufferSize == 0)
			file->bufferValid = 0;

		if((*readCount) == 0)
		{
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