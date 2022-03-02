#include"FS.hpp"
#include"Memory.hpp"

using FS::File;
using FS::Directory;
using FS::DirEntry;

const u16 CLUSTER_EMPTY	=	0x000;
const u16 CLUSTER_LAST	=	0xFF8;

const u16 ENTRY_REMOVED	=	0xE5;
const u16 ENTRY_LAST	=	0x00;

namespace FAT
{
	struct Info;
	struct Cache;
}

namespace FS
{
	// struct Cache;

	struct Directory
	{
		u32 rootCluster;

		FAT::Cache* buffer;
	};

	struct File
	{
		u32 rootCluster;
		u32 parentCluster;
		u32 parentOffset;

		FAT::Cache* buffer;
	};

	// struct File
	// {
	// 	bool dirty;
	// 	u32 firstCluster;
	// 	u32 currentCluster;
	// 	u32 totalDataOffset; // Total data offset, not position in buffer
	// 	u32 size;
	// 	bool sizeDirty; // TODO update file entry
	// 	u8 bufferValid;
	// 	u32 bufferSize;
	// 	u8* buffer;
	// 	u32 directoryCluster;

	// 	File(u32 bufferSize, u32 firstCluster, u32 size, u32 dirClust) : dirty(false),
	// 		firstCluster(firstCluster), currentCluster(firstCluster), totalDataOffset(0),
	// 		size(size), bufferValid(false), bufferSize(bufferSize), directoryCluster(dirClust)
	// 	{
	// 		buffer = new u8[bufferSize];
	// 	}

	// 	~File()
	// 	{
	// 		delete[] buffer;
	// 	}

	// 	Status Flush(FAT::Info* info);
	// 	Status ReadNextCluster(FAT::Info* info, bool resizeIfNeeded);
	// 	Status ReadAtOffset(FAT::Info* info, u32 newTotalOffset, bool resizeIfNeeded);
	// };

	// struct Directory
	// {
	// 	bool dirty;
	// 	u32 firstCluster;
	// 	u32 currentCluster;
	// 	u32 dataOffset;
	// 	u8 bufferValid;
	// 	u32 bufferSize;
	// 	u8* buffer;

	// 	Directory(u32 bufferSize, u32 firstCluster) : dirty(false), firstCluster(firstCluster), currentCluster(firstCluster), dataOffset(0), bufferValid(false), bufferSize(bufferSize)
	// 	{
	// 		buffer = new u8[bufferSize];
	// 	}

	// 	~Directory()
	// 	{
	// 		delete[] buffer;
	// 	}

	// 	Status Flush(FAT::Info* info);
	// };
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
	 * Cache
	 */

	struct Cache
	{
		u32 refCount;
		u32 cluster;
		bool dirty;

		u32 bufferSize;
		u8* buffer;
	};

	Container::LinkedList<Cache*> cacheObjects;

	Cache* GetCache(u32 cluster)
	{
		for(auto v : cacheObjects)
		{
			if(v->cluster == cluster)
			{
				v->refCount++;
				return v;
			}
		}

		if(cluster == 0)
		{
			// Read root
		}
		else
		{
			// Read data
		}
	}

	/*
	 *  Helper functions
	 */

	int StrFindLastOf(char* str, char c)
	{
		unsigned len = strlen(str);

		int pos = -1;
		for(unsigned a = 0; a < len; a++)
		{
			if(str[a] == c)
				pos = a;
		}

		return pos;
	}

	void LFNToF83(char* lfn, char* filename)
	{
		char* ext = filename + 8;

		auto lfnLen = strlen(lfn);
		auto dotPos = StrFindLastOf(lfn, '.');
		if(dotPos < 0 || dotPos + 1 >= lfnLen)
			dotPos = lfnLen;

		for(unsigned a = 0; a < 8+3; ++a)
		{
			if(a < dotPos && a < 8)
			{
				filename[a] = toupper(lfn[a]);
			}
			else if(a >= 8 && a - 8 < lfnLen - dotPos)
			{
				ext[a - 8] = toupper(lfn[dotPos + a - 8 + 1]);
			}
			else
			{
				if(a < 8)
					filename[a] = ' ';
				else
					ext[a - 8] = ' ';
			}
		}
	}

	void F83ToLFN(char* filename, char* lfn)
	{
		char* ext = filename + 8;
		unsigned lfnPos = 0;

		for(unsigned a = 0; a < 8; ++a)
		{
			if(filename[a] == ' ')
				break;
			lfn[lfnPos++] = filename[a];
		}

		bool isExt = false;
		for(unsigned a = 0; a < 3; ++a)
		{
			if(ext[a] == ' ')
				break;

			if(!isExt)
			{
				lfn[lfnPos++] = '.';
				isExt = true;
			}

			lfn[lfnPos++] = ext[a];
		}
	}

	u32 NextCluster(Info* fs, u16 cluster)
	{
		u32 nextCluster;

		if(fs->type == Type::FAT12)
		{
			u32 fatOffset = cluster + (cluster >> 1);
			nextCluster = *(u16*)(fs->fat + fatOffset);

			if(cluster & 1)
				nextCluster = nextCluster >> 4;
			else
				nextCluster = nextCluster & 0xfff;

			if(nextCluster >= 0xFF8) // Last one...
				nextCluster = 0;
		}
		else if(fs->type == Type::FAT16)
		{
			nextCluster = *((u16*)fs->fat + cluster);

			if(nextCluster >= 0xFFF8) // Last one...
				nextCluster = 0;
		}
		else
		{
			nextCluster = 0;
		}

		return nextCluster;
	}

	u16 GetRootSector(Info* info)
	{
		auto bpb = info->bpb;
		return bpb->hiddenSectors + bpb->reservedSectors + bpb->sectorsPerFat * bpb->fatsCount;
	}

	u16 GetFirstDataSector(Info* info)
	{
		auto bpb = info->bpb;
		return GetRootSector(info) + (bpb->rootEntriesCount * sizeof(FAT16_DirEntry) / bpb->bytesPerSector);
	}

	u16 GetSectorToRead(Info* info, u16 cluster, bool isRoot)
	{
		auto bpb = info->bpb;
		u16 sectorToRead = GetRootSector(info) + cluster * bpb->sectorsPerCluster;// getFirstDataSector(h) + cluster;
		if(!isRoot)
			sectorToRead += bpb->rootEntriesCount * sizeof(FAT16_DirEntry) / bpb->bytesPerSector - 2;

		return sectorToRead;
	}

	u16 GetNextCluster(Info* info, u16 cluster)
	{
		if(cluster >= CLUSTER_LAST)
			return CLUSTER_LAST;

		u16 offset = cluster + (cluster >> 1);
		u16 value = *(u16*)(info->fat + offset);

		if(cluster & 0x0001)
			return value >> 4;
		else
			return value & (u16)0x0FFF;
	}

	bool IsRoot(Directory* dir)
	{
		return dir->firstCluster == 0;
	}

	bool SetCluster(Info* info, u16 cluster, u16 nextCluster)
	{
		auto lastNextCluster = GetNextCluster(info, cluster);
		if(lastNextCluster < CLUSTER_LAST && lastNextCluster != 0x000)
			return false;

		u16 offset = cluster + (cluster >> 1);
		u16* value = (u16*)(info->fat + offset);

		if(cluster & 0x0001)
			*value = (*value & 0x000f) | (nextCluster << 4);
		else
			*value = (*value & 0xf000) | (nextCluster);

		//printf("New value: %04x (%d, %d)\n", *value, cluster, nextCluster);

		// TODO mark sector as dirty
		/*BPB* bpb = &h->bpb;
		for(unsigned a = 0; a < bpb->fatsCount; ++a)
		{
			h->file.seekp( (bpb->hiddenSectors + bpb->reservedSectors + a * bpb->sectorsPerFat) * bpb->bytesPerSector);
			h->file.write((char*)h->fat, bpb->sectorsPerFat * bpb->bytesPerSector);
		}*/

		// Write whole FAT... FIXME
		for(unsigned fatId = 0; fatId < info->bpb->fatsCount; fatId++)
		{
			u32 fatSector = info->bpb->hiddenSectors + info->bpb->reservedSectors + (info->bpb->sectorsPerFat * fatId);
			for(unsigned a = 0; a < info->bpb->sectorsPerFat; a++)
				info->part->Write(fatSector + a, info->fat + a * 512);
		}

		return true;
	}

	Status AddCluster(Info* info, u32 parentCluster, u32* nextCluster)
	{
		if(parentCluster > 0 && GetNextCluster(info, parentCluster) < CLUSTER_LAST)
			return Status::Undefined;

		auto bpb = info->bpb;
		u32 freeClusterIndex = 0;
		u32 totalFatEntries = bpb->sectorsPerFat * bpb->bytesPerSector * 8 / 12; // FIXME ?
		for(; freeClusterIndex < totalFatEntries; freeClusterIndex++)
		{
			u16 offset = freeClusterIndex + (freeClusterIndex >> 1);
			u16 value = *(u16*)(info->fat + offset);
			if(freeClusterIndex & 1)
				value = value >> 4;
			else
				value = value & 0xfff;

			if(value == CLUSTER_EMPTY)
				break;
		}

		if(freeClusterIndex >= totalFatEntries)
			return Status::Undefined;

		if(parentCluster > 0)
			SetCluster(info, parentCluster, freeClusterIndex);
		SetCluster(info, freeClusterIndex, CLUSTER_LAST);

		// Copy old data from cluster?
		/*char tmp[512];
		memset(tmp, 0, 512);
		auto sector = GetSectorToRead(info, freeClusterIndex, false);
		memcpy(h->buffer + sector * 512, tmp, 512);*/

		if(nextCluster)
			*nextCluster = freeClusterIndex;
		return Status::Success;
	}

	bool IsFreeOrRemoved(FAT16_DirEntry* entry)
	{
		return (entry->name[0] == 0 || entry->name[0] == ' ' || entry->name[0] == ENTRY_REMOVED);
	}

	/*
	 *  Filesystem operations
	 */

	Status Probe(Block::BlockPartition* part)
	{
		u8 buffer[512];

		Print("Probing FAT16...\n");
		if(part->Read(0, buffer))
			return Status::Undefined;

		if(buffer[0] == 0xEB && buffer[2] == 0x90)
			return Status::Success;

		return Status::Undefined;
	}

	struct Parameters
	{
		u8		oem[8];
		u16		bytesPerSector;
		u8		sectorsPerCluster;
		u16		reservedSectors;
		u8		fatsCount;
		u16		rootEntriesCount;
		u16		totalSectors;
		u8		mediaType;
		u16		sectorsPerFat;
		u16		sectorsPerTrack;
		u16		headsCount;
		u32		hiddenSectors;
		// EBR
		u8		driveNumber;
		u32		volumeId;
		char*	label;
		u8		systemId[8];
		u8		*bootloader;
		bool	bootable;

		Parameters()
		{
			memset(this, 0, sizeof(*this));
		}
	};

	Status FormatFAT(Block::BlockPartition* part, BPB* bpb, Parameters* p)
	{
		char tmp_fat[512] = { 0 };
		//memset(h->fat, 0, bpb->sectorsPerFat * bpb->bytesPerSector);
		tmp_fat[0] = tmp_fat[1] = tmp_fat[2] = 0xFF; // mark as reserved
		tmp_fat[0] = 0xF0;

		unsigned fat_sector = bpb->reservedSectors + bpb->hiddenSectors;
		part->Write(fat_sector, (u8*)tmp_fat);

		memset(tmp_fat, 0, 512);
		for(unsigned a = 1; a <  bpb->sectorsPerFat; a++)
			part->Write(fat_sector + a, (u8*)tmp_fat);

		//memset(h->fatDirty, 1, p->sectorsPerCluster);

		return Status::Success;
	}

	Status FormatRoot(Block::BlockPartition* part, BPB* bpb, Parameters* p)
	{
		char buffer[p->bytesPerSector];
		memset(buffer, 0, p->bytesPerSector);

		Info info;
		info.bpb = bpb;
		//memcpy(&info.bpb, bpb, sizeof(*bpb));

		//unsigned rootSector = (p->reservedSectors + p->hiddenSectors) + (p->fatsCount * p->sectorsPerFat);
		unsigned rootSector = GetRootSector(&info);
		unsigned rootSectorsCount = p->rootEntriesCount * sizeof(FAT16_DirEntry) / p->bytesPerSector;

		// First volume directory label
		FAT16_DirEntry* label = (FAT16_DirEntry*)buffer;
		//memcpy(label->name, FAT12::Filename(p->label).value, 11); // TODO: fixme
		label->attributes = u8(Attribute::VolumeLabel | Attribute::Archive);

		*(u16*)&label->modificationTime = 0x021c;
		*(u16*)&label->modificationDate = 0x4c21;

		for(unsigned a = 0; a < rootSectorsCount; ++a)
		{
			//memcpy(h->buffer + (rootSector + a) * 512, buffer, 512);
			part->Write(rootSector + a, (u8*)buffer);

			if(a == 0)
			{
				memset(buffer, 0, sizeof(FAT16_DirEntry));
			}
		}

		return Status::Success;
	}

	Status Format(Block::BlockPartition* part, void* params)
	{
		Parameters x;
		if(params != nullptr)
			x = *(Parameters*)params;

		if(x.totalSectors == 0)
			x.totalSectors = part->lbaCount;

		if(x.rootEntriesCount == 0)
			x.rootEntriesCount = 288;

		if(x.bytesPerSector == 0)
			x.bytesPerSector = 512;

		if(!x.label || strlen(x.label) == 0)
			x.label = "baby-cFS";

		//p->reservedSectors++;

		//auto usableSectors = p->totalSectors - p->reservedSectors - (p->rootEntriesCount * sizeof(DirectoryEntry) / 512);
		//auto fatSize =

		//auto bpb = info->bpb;
		BPB bpb;
		memset(&bpb, 0xcc, sizeof(BPB));

		// Init BPB

		//memcpy(&bpb._unused, "\xeb\x3c\x90", 3);
		memcpy(&bpb._unused1, "\xeb\xfe\x90", 3);

		memcpy(bpb.oem, "MSDOS5.0", 8);
		bpb.bytesPerSector = x.bytesPerSector;
		bpb.sectorsPerCluster = 1; // TODO
		bpb.reservedSectors = 1 + x.reservedSectors;
		bpb.fatsCount = 2;
		bpb.rootEntriesCount = 14 * x.bytesPerSector / sizeof(FAT16_DirEntry); // 224
		bpb.totalSectors = x.totalSectors;
		bpb.mediaType = 0xF0; // TODO

		unsigned totalUsableSectors = bpb.totalSectors - bpb.reservedSectors - (bpb.rootEntriesCount * sizeof(FAT16_DirEntry) / x.bytesPerSector);

		//bpb.sectorsPerFat = std::ceil((3 * totalUsableSectors) / (6 + (1024.0 * bpb.sectorsPerCluster))); // FIXME
		int dividend = (3 * totalUsableSectors);
		int divisor = (6 + (1024 * bpb.sectorsPerCluster));
		bpb.sectorsPerFat = (dividend + divisor - 1) / divisor;

		bpb.sectorsPerTrack = 18; // TODO
		bpb.headsCount = 2; // TODO
		bpb.hiddenSectors = 0;
		bpb.totalSectorsHigh = 0;

		// Init EBPB
		bpb.driveNumber = 0;
		bpb._unused2 = 0;
		bpb.signature = 0x29; // TODO

		*(u32*)&bpb.volumeId = 0x6F4C6F59; //0x596F4C6F;
		//memcpy(bpb.label, FAT12::Filename(p->label).value, 11);

		memset(bpb.label, ' ', 11);
		memcpy(bpb.label, x.label, strlen(x.label));

		memcpy(bpb.systemId, "MSDOS5.0", 8);
		*(u16*)&bpb.bootSignature = (x.bootloader != nullptr) ? 0xAA55 : 0x0000;

		part->Write(0, (u8*)&bpb);

		// if(p->bootloader != nullptr)
		// {
		// 	memcpy(((char*)h->bpb) + offsetof(BPB, _unused1), p->bootloader + offsetof(BPB, _unused1), sizeof(BPB::_unused1));
		// 	memcpy(((char*)h->bpb) + offsetof(BPB, bootCode), p->bootloader + offsetof(BPB, bootCode), sizeof(BPB::bootCode));

		// 	printf("Bootcode: ");
		// 	for(int a = 0; a < sizeof(BPB::bootCode); ++a)
		// 		printf("%02x", p->bootloader[offsetof(BPB, bootCode)+a]);
		// }

		// h->fat = (char*)h->buffer + (bpb.reservedSectors + bpb.hiddenSectors) * bpb.bytesPerSector;

		// TODO init FAT
		FormatFAT(part, &bpb, &x);
		FormatRoot(part, &bpb, &x);

		return Status::Success;
	}

	Status Mount(Block::BlockPartition* part, void** fs)
	{
		if(Probe(part) != Status::Success)
		{
			*fs = nullptr; // FIXME
			return Status::Undefined;
		}

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

		info->firstRootSector = GetRootSector(info);
		// info->firstRootSector = info->bpb->reservedSectors
		// 			+ info->bpb->sectorsPerFat * info->bpb->fatsCount;
		// info->firstDataSector = info->firstRootSector + info->bpb->rootEntriesCount * 32 / 512;
		info->firstDataSector = GetFirstDataSector(info);
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
		return Status::Success;
	}

	Status Unmount(Block::BlockPartition* part, void** fs)
	{
		if(!fs)
			return Status::Success;

		Info* info = (Info*)(*fs);
		Memory::Free(info->bpb);
		Memory::Free(info->fat);
		Memory::Free(info);

		(*fs) = nullptr;
		return Status::Success;
	}

	Status LabelGet(void* fs, char* buffer, u32* bufferSize)
	{
		if(!fs)
			return Status::Undefined;

		return Status::Undefined;
	}

	Status LabelSet(void* fs, char* buffer)
	{
		if(!fs)
			return Status::Undefined;

		return Status::Undefined;
	}

	Status DirectoryOpenRoot(void* fs, Directory** dir)
	{
		if(!fs)
			return Status::Undefined;

		Info* info = (Info*)fs;
		u32 bufferSize = info->part->device->drv->BlockSize(info->part->device->drvPriv) * info->bpb->sectorsPerCluster;
		*dir = new Directory(bufferSize, 0);

		return Status::Success;
	}

	Status DirectoryClose(void* fs, Directory** dir)
	{
		if(!fs || !dir || !(*dir))
			return Status::Undefined;

		(*dir)->Flush((Info*)fs);

		delete (*dir);
		(*dir) = nullptr;

		return Status::Success;
	}

	Status DirectoryReadInternal(Info* info, Directory* dir, DirEntry* entry)
	{
		auto part = info->part;

		if(dir->bufferValid)
			dir->dataOffset += 32;

		if(dir->dataOffset == 0 || dir->dataOffset == 512 * info->bpb->sectorsPerCluster)
		{
			dir->Flush(info);

			if(dir->dataOffset == 512 * info->bpb->sectorsPerCluster)
			{
				if(dir->firstCluster == 0)
					dir->currentCluster++;
				else
					dir->currentCluster = NextCluster(info, dir->currentCluster);

				dir->dataOffset = 0;
			}

			// Check if eof of root
			if(dir->firstCluster == 0 && dir->currentCluster >= (info->bpb->rootEntriesCount * 32 / (512 * info->bpb->sectorsPerCluster)))
			{
				dir->bufferValid = 0;
				return Status::EndOfFile;
			}
			// Check if eof of non-root
			if(dir->firstCluster != 0 && dir->currentCluster == 0)
			{
				dir->bufferValid = 0;
				return Status::EndOfFile;
			}

			u32 firstSector = GetSectorToRead(info, dir->currentCluster, IsRoot(dir));
			// u32 firstSector = (dir->firstCluster == 0)
			// 		? (info->firstRootSector + dir->currentCluster * info->bpb->sectorsPerCluster)
			// 		: (info->firstDataSector + (dir->currentCluster - 2) * info->bpb->sectorsPerCluster);

			for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
			{
				part->Read(firstSector + a, dir->buffer + 512 * a);
			}

			dir->bufferValid = 1;
		}

		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

		// printf("_ read dir, cluster: %d, offset: %d\n", dir->currentCluster, dir->dataOffset);

		entry->isDirectory = fatEntry->attributes & (u8)Attribute::Directory;
		entry->isSymlink = 0;
		//entry->isValid = (fatEntry->name[0] != 0 && fatEntry->name[0] != ' ' && fatEntry->name[0] != 0xE5); // < deleted
		entry->isValid = !IsFreeOrRemoved(fatEntry);
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

		return Status::Success;
	}

	Status DirectoryRead(void* fs, Directory* dir, DirEntry* entry)
	{
		if(!fs)
			return Status::Undefined;

		Info* info = (Info*)fs;
		Status s;

		do
		{
			s = DirectoryReadInternal(info, dir, entry);
		} while(s == Status::Success && !entry->isValid);

		return s;
	}

	Status DirectoryRewind(void* fs, Directory* dir)
	{
		if(!fs)
			return Status::Undefined;

		// if(dir->currentCluster != dir->firstCluster) // bufferValid must be zeroed
		// or otherwise, reading directory will skip first entry :(
		{
			dir->Flush((Info*)fs);
			dir->currentCluster = dir->firstCluster;
			dir->bufferValid = 0;
		}

		dir->dataOffset = 0;

		return Status::Success;
	}

	Status DirectoryFollow(void* fs, Directory* dir)
	{
		if(!fs)
			return Status::Undefined;

		dir->Flush((Info*)fs);

		Print("Changing directory...\n");
		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

		if(!(fatEntry->attributes & (u8)Attribute::Directory))
		{
			Print("Not a directory!!\n");
			return Status::Undefined;
		}

		dir->firstCluster = dir->currentCluster = fatEntry->cluster;
		dir->dataOffset = 0;
		dir->bufferValid = 0;

		return Status::Success;
	}

	Status ExtendDirectoryIfNoFreeEntries(Info* info, Directory* dir)
	{
		DirEntry entry;
		Status s;

		DirectoryRewind(info, dir);
		do
		{
			s = DirectoryReadInternal(info, dir, &entry);
		} while(s == Status::Success && entry.isValid);

		if(s != Status::Success)
		{
			if(IsRoot(dir))
				return Status::EndOfFile;

			auto currentCluster = dir->firstCluster;
			auto nextCluster = currentCluster;

			do
			{
				currentCluster = nextCluster;
				nextCluster = GetNextCluster(info, currentCluster);
			} while(nextCluster != CLUSTER_LAST);

			AddCluster(info, currentCluster, &nextCluster);

			u32 clusterSize = 512 * info->bpb->sectorsPerCluster;
			memset(dir->buffer, 0, dir->bufferSize);
			auto sector = GetSectorToRead(info, nextCluster, false);
			info->part->Write(sector, (u8*)dir->buffer);

			Print("Directory extended~!\n");
		}
		else
			Print("Directory not extended~!\n");

		return Status::Success;
	}

	Status DirectoryAddEntry(Info* info, Directory* dir, const FAT16_DirEntry* fatEntryTemplate)
	{
		DirEntry entry;
		Status s;

		if(ExtendDirectoryIfNoFreeEntries(info, dir) != Status::Success)
			return Status::Undefined;

		DirectoryRewind(info, dir);
		do
		{
			s = DirectoryReadInternal(info, dir, &entry);
			// Print("Adding entry: Cluster: %d, Offset: %d\r", dir->currentCluster, dir->dataOffset);
			// Timer::Delay(100);
		} while(s == Status::Success && entry.isValid);

		Print("Adding entry: Cluster: %d, Offset: %d - \"%s\"\n", dir->currentCluster, dir->dataOffset, fatEntryTemplate->name);

		FAT16_DirEntry* newEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);
		memcpy(newEntry, fatEntryTemplate, sizeof(*fatEntryTemplate));

		// auto dirLBA = GetSectorToRead(info, dir->currentCluster, IsRoot(dir));
		// info->part->Write(dirLBA, dir->buffer);

		dir->dirty = true;
		// DirectoryFlush(fs, dir);

		return Status::Success;
	}

	Status DirectoryCreate(void* fs, Directory* dir, char* name)
	{
		if(!fs)
			return Status::Undefined;

		Info* info = (Info*)fs;

		Status s;

		u32 newDirCluster;
		AddCluster(info, 0, &newDirCluster);

		// Clear data in new cluster
		auto newDirSector = GetSectorToRead(info, newDirCluster, false);
		char tmp[512] = { 0 };
		for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
			info->part->Write(newDirSector + a, (u8*)tmp);

		//FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);
		FAT16_DirEntry parentEntry;
		LFNToF83(name, (char*)parentEntry.name);
		parentEntry.attributes = (u8)Attribute::Directory;
		parentEntry.cluster = newDirCluster;
		parentEntry.size = 0;

		DirectoryAddEntry(info, dir, &parentEntry);
		dir->Flush(info);

		// Flush
		// u32 dirFirstCluster = dir->firstCluster;
		// u32 dirSector = GetSectorToRead(info, dir->currentCluster, IsRoot(dir));
		// info->part->Write(dirSector, dir->buffer);

		// Create directories
		// Directory newDir = {
		// 	.firstCluster = newDirCluster,
		// 	.currentCluster = newDirCluster,
		// 	.dataOffset = 0,
		// 	.bufferValid = false,
		// 	.bufferSize = 512,
		// };
		Directory* newDir;
		DirectoryOpenRoot(info, &newDir); // TODO: check status
		newDir->firstCluster = newDir->currentCluster = newDirCluster;

		FAT16_DirEntry newEntry;
		newEntry._reserved = 0x4D;
		newEntry._unused = 0x54;
		newEntry.createDate = Date { .year = 22, .month = 1, .day = 1 };
		newEntry.createTime = Time { .hour = 0, .minutes = 0, .seconds = 0 };
		newEntry.modificationDate = Date { .year = 22, .month = 1, .day = 1 };
		newEntry.modificationTime = Time { .hour = 0, .minutes = 0, .seconds = 0 };
		newEntry.accessDate = Date { .year = 22, .month = 1, .day = 1 };
		newEntry.size = 0;
		newEntry.attributes = (u8)Attribute::Directory;
		newEntry.clusterHigh = 0;
		// .
		LFNToF83(".", (char*)newEntry.name);
		newEntry.cluster = newDirCluster;
		DirectoryAddEntry(info, newDir, &newEntry);
		newDir->Flush(info);
		// ..
		LFNToF83("..", (char*)newEntry.name);
		newEntry.cluster = dir->firstCluster;
		DirectoryAddEntry(info, newDir, &newEntry);

		newDir->Flush(info);

		return Status::Success;
	}

	Status DirectoryRemove(void* fs, Directory* dir)
	{
		if(!fs)
			return Status::Undefined;

		return Status::Undefined;
	}

	Status FileCreate(void* fs, Directory* dir, char* name)
	{
		if(!fs)
			return Status::Undefined;

		Info* info = (Info*)fs;

		FAT16_DirEntry newEntry;
		LFNToF83(name, (char*)newEntry.name);
		newEntry._reserved = 0x4D;
		newEntry._unused = 0x54;
		newEntry.createDate = Date { .year = 22, .month = 1, .day = 1 };
		newEntry.createTime = Time { .hour = 0, .minutes = 0, .seconds = 0 };
		newEntry.modificationDate = Date { .year = 22, .month = 1, .day = 1 };
		newEntry.modificationTime = Time { .hour = 0, .minutes = 0, .seconds = 0 };
		newEntry.accessDate = Date { .year = 22, .month = 1, .day = 1 };
		newEntry.size = 0;
		newEntry.attributes = (u8)0;
		newEntry.clusterHigh = 0;
		newEntry.cluster = 0;
		DirectoryAddEntry(info, dir, &newEntry);
		dir->Flush(info);

		return Status::Success;
	}

	Status FileDelete(void* fs, Directory* dir)
	{
		if(!fs)
			return Status::Undefined;

		return Status::Undefined;
	}

	Status FileOpen(void* fs, Directory* dir, File** file)
	{
		if(!fs)
			return Status::Undefined;

		// printf("# Opening file\n");

		Info* info = (Info*)fs;
		u32 bufferSize = info->part->device->drv->BlockSize(info->part->device->drvPriv) * info->bpb->sectorsPerCluster;
		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

		(*file) = new File(bufferSize, (fatEntry->clusterHigh << 16) | fatEntry->cluster, fatEntry->size, dir->firstCluster);

		return Status::Success;
	}

	Status FileClose(void* fs, File** file)
	{
		if(!fs)
			return Status::Undefined;

		Print("# Flushing file\n");
		(*file)->Flush((Info*)fs);

		Print("# Closing file\n");
		delete (*file);
		(*file) = nullptr;

		return Status::Success;
	}

	Status FileRead(void* fs, File* file, u8* buffer, u32 bufferSize, u32* readCount)
	{
		if(!fs)
			return Status::Undefined;

		Info* info = (Info*)fs;
		auto part = info->part;

		(*readCount) = 0;

		if(file->totalDataOffset >= file->size)
		{
			Print("EoF\n");
			// return Status::EndOfFile;
		}

		if(!file->bufferValid)
		{
			if(file->totalDataOffset > 0)
			{
				file->currentCluster = NextCluster(info, file->currentCluster);

				if(file->currentCluster == 0)
				{
					Print("Last cluster\n");
					return Status::EndOfFile;
				}
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
			Status status;
			u32 subread;

			part->device->drv->Lock(part->device->drvPriv);
			for(unsigned a = 0; a < bufferSize; )
			{
				u32 curSize = (bufferSize - (*readCount) > file->bufferSize ? file->bufferSize : bufferSize - (*readCount));

				status = FileRead(fs, file, buffer + (*readCount), curSize, &subread);
				Print("Read: %u\n", subread);
				a += subread;
				(*readCount) += subread;

				if(status != Status::Success)
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
			// if(file->totalDataOffset >= file->size)
			// 	break;

			buffer[a] = file->buffer[offset + a];

			file->totalDataOffset++;
			(*readCount)++;
		}

		if(file->totalDataOffset % file->bufferSize == 0)
			file->bufferValid = 0;

		if((*readCount) == 0)
		{
			Print("_EoF_\n");
			return Status::EndOfFile;
		}

		return Status::Success;
	}

	void FilePutByte(Info* info, File* file, u8 byte)
	{
		file->dirty = true;

		Print("#   put byte: \"%x\" - %d %d\n", byte, file->currentCluster, file->totalDataOffset);

		if((file->totalDataOffset > 0 && file->totalDataOffset % file->bufferSize == 0) || !file->bufferValid)
		{
			Print("#  ReadNextCluster: %d - %d,%d\n", file->totalDataOffset, file->firstCluster, file->currentCluster);
			file->ReadNextCluster(info, true);
		}

		file->buffer[file->totalDataOffset % file->bufferSize] = byte;
		file->totalDataOffset++;

		if(file->totalDataOffset > file->size)
		{
			file->sizeDirty = true;
			file->size = file->totalDataOffset;
		}
	}

	Status FileWrite(void* fs, File* file, u8* buffer, u32 bufferSize, u32* writeCount)
	{
		if(!fs || !file)
			return Status::Undefined;

		Print("# Write file: %d\n", bufferSize);

		(*writeCount) = 0;
		for(unsigned a = 0; a < bufferSize; a++)
		{
			FilePutByte((Info*)fs, file, buffer[a]);
			(*writeCount)++;
		}

		return Status::Success;
	}

	Status FileSetPointer(void* fs, File* file, u32 offset)
	{
		if(!fs)
			return Status::Undefined;

		return Status::Undefined;
	}

	Status FileGetPointer(void* fs, File* file, u32* offset)
	{
		if(!fs)
			return Status::Undefined;

		return Status::Undefined;
	}

	FS::FSInfo info =
	{
		.name = "FAT",
		.Probe = Probe,
		.Format = Format,
		.Mount = Mount,
		.Unmount = Unmount,
		.LabelGet = LabelGet,
		.LabelSet = LabelSet,
		.DirectoryOpenRoot = DirectoryOpenRoot,
		.DirectoryClose = DirectoryClose,
		.DirectoryRead = DirectoryRead,
		.DirectoryRewind = DirectoryRewind,
		.DirectoryFollow = DirectoryFollow,
		.DirectoryCreate = DirectoryCreate,
		.DirectoryRemove = DirectoryRemove,
		.FileCreate = FileCreate,
		.FileDelete = FileDelete,
		.FileOpen = FileOpen,
		.FileClose = FileClose,
		.FileRead = FileRead,
		.FileWrite = FileWrite,
		.FileSetPointer = FileSetPointer,
		.FileGetPointer = FileGetPointer,
	};

	bool Init()
	{
		FS::Register(&info);

		return true;
	}
}

Status FS::File::Flush(FAT::Info* info)
{
	if(sizeDirty)
	{
		// Update dir entry
		FS::DirEntry entry;
		Directory* dir = new Directory(512, directoryCluster);
		while(FAT::DirectoryReadInternal(info, dir, &entry) == Status::Success)
		{
			FAT::FAT16_DirEntry* fatEntry = (FAT::FAT16_DirEntry*)(dir->buffer + dir->dataOffset);

			if(fatEntry->cluster == this->firstCluster || (this->firstCluster == 0 && strcmp()))
			{
				Print("Found entry to update size~!\n");
				fatEntry->size = this->size;
				dir->dirty = true;
				break;
			}
			else if(fatEntry->cluster != 0)
				Print("- %d ?= %d - %s\n", fatEntry->cluster, this->firstCluster, entry.name);
		}

		Print("Updated size: %d\n", this->size);
		dir->Flush(info);
		sizeDirty = false;
	}

	if(!dirty || !bufferValid)
		return Status::Success;

	u32 sector = FAT::GetSectorToRead(info, currentCluster, false);

	if(sector >= info->part->lbaCount)
	{
		Print("Invalid sector (%d) for cluster %d\n", sector, currentCluster);
		return Status::EndOfFile;
	}

	Print("Sector: %d\n", sector);

	for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
	{
		info->part->Write(sector + a, buffer + 512 * a);
	}

	dirty = false;

	return Status::Success;
}

Status FS::File::ReadNextCluster(FAT::Info* info, bool resizeIfNeeded)
{
	if(Flush(info) != Status::Success)
		return Status::IOError;

	if(bufferValid)
	{
		u32 nextCluster = FAT::GetNextCluster(info, currentCluster);

		if(nextCluster == CLUSTER_LAST)
		{
			if(!resizeIfNeeded)
				return Status::EndOfFile;
			else
			{
				if(FAT::AddCluster(info, currentCluster, &nextCluster) != Status::Success)
					return Status::Undefined;

				char tmp[512] = { 0 };
				u32 sector = FAT::GetSectorToRead(info, nextCluster, false);
				for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
					info->part->Write(sector + a, (u8*)tmp);
			}
		}

		totalDataOffset += bufferSize;
		totalDataOffset -= totalDataOffset % bufferSize;

		currentCluster = nextCluster;
	}
	else
	{
		if(firstCluster == 0)
		{
			if(FAT::AddCluster(info, 0, &firstCluster) != Status::Success)
				return Status::Undefined;

			char tmp[512] = { 0 };
			u32 sector = FAT::GetSectorToRead(info, firstCluster, false);
			for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
				info->part->Write(sector + a, (u8*)tmp);

			currentCluster = firstCluster;
		}
	}

	u32 sector = FAT::GetSectorToRead(info, currentCluster, false);
	for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
		info->part->Read(sector + a, buffer + 512 * a);

	bufferValid = true;

	return Status::Success;
}

Status FS::File::ReadAtOffset(FAT::Info* info, u32 newTotalOffset, bool resizeIfNeeded)
{
	if(Flush(info) != Status::Success)
		return Status::IOError;
}

Status FS::Directory::Flush(FAT::Info* info)
{
	if(!dirty)
		return Status::Success;

	auto sector = GetSectorToRead(info, currentCluster, FAT::IsRoot(this));
	for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
		info->part->Write(sector + a, buffer + 512 * a);

	dirty = false;
}