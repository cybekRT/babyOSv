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
	struct Directory
	{
		u32 rootCluster;

		u32 bufferOffset;
		FAT::Cache* cache;
	};

	struct File
	{
		u32 rootCluster;
		u32 parentCluster;
		u32 parentOffset;
		bool parentNeedsUpdate;

		u32 totalOffset;
		u32 totalSize;
		FAT::Cache* cache;
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

		Container::LinkedList<Cache*> cacheObjects;
	};

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

	u32 GetNextCluster(Info* fs, u16 cluster)
	{
		u32 nextCluster;

		if(cluster < 2)
		{
			Print("Invalid cluster in chain... %d\n", cluster);
			return 0;
		}

		if(fs->type == Type::FAT12)
		{
			u32 fatOffset = cluster + (cluster >> 1);
			nextCluster = *(u16*)(fs->fat + fatOffset);

			if(cluster & 1)
				nextCluster = nextCluster >> 4;
			else
				nextCluster = nextCluster & 0xfff;

			Print("Cluster chain: %d -> %d\n", cluster, nextCluster);

			if(nextCluster >= 0xFF8) // Last one...
				nextCluster = 0;
		}
		else if(fs->type == Type::FAT16)
		{
			nextCluster = *((u16*)fs->fat + cluster);

			if(nextCluster >= 0xFFF8) // Last one...
				nextCluster = 0;
		}
		else // FAT32
		{
			Print("No FAT32 support yet...");
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

	bool IsRoot(Directory* dir)
	{
		return dir->rootCluster == 0;
	}

	bool SetCluster(Info* info, u16 cluster, u16 nextCluster)
	{
		// FIXME: ???
		// auto lastNextCluster = GetNextCluster(info, cluster);
		// if(lastNextCluster < CLUSTER_LAST && lastNextCluster != 0x000)
		// 	return false;

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
		if(parentCluster > 0 && GetNextCluster(info, parentCluster) != 0)//< CLUSTER_LAST)
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

	Cache* GetCache(Info* info, u32 cluster)
	{
		for(auto v : info->cacheObjects)
		{
			if(v->cluster == cluster)
			{
				v->refCount++;
				return v;
			}
		}

		// TODO: mutex
		Cache* cache = new Cache();
		cache->refCount = 1;
		cache->cluster = cluster;
		cache->dirty = 0;

		if(cluster == 0)
		{
			// Read root
			auto sectorsCount = info->bpb->rootEntriesCount * sizeof(FAT16_DirEntry) / 512;
			u32 sector = GetRootSector(info);

			cache->bufferSize = sectorsCount * 512;
			cache->buffer = new u8[cache->bufferSize];
			for(unsigned a = 0; a < sectorsCount; a++)
				info->part->Read(sector + a, cache->buffer + a * 512);
		}
		else
		{
			// Read data
			u32 sector = GetSectorToRead(info, cluster, false);

			cache->bufferSize = 512 * info->bpb->sectorsPerCluster;
			cache->buffer = new u8[cache->bufferSize];
			for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
				info->part->Read(sector + a, cache->buffer + a * 512);
		}

		info->cacheObjects.PushBack(cache);
		return cache;
	}

	void FreeCache(Info* info, Cache* cache)
	{
		if(!cache)
			return;

		cache->refCount--;
		// TODO: mutex

		if(cache->refCount <= 0 && cache->dirty)
		{
			u32 sector = GetSectorToRead(info, cache->cluster, cache->cluster == 0);
			for(unsigned a = 0; a < cache->bufferSize; a+=512)
			{
				info->part->Write(sector + a / 512, cache->buffer + a);
			}
		}
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
		tmp_fat[0] = tmp_fat[1] = tmp_fat[2] = 0xFF; // mark as reserved
		tmp_fat[0] = 0xF0;

		unsigned fat_sector = bpb->reservedSectors + bpb->hiddenSectors;
		part->Write(fat_sector, (u8*)tmp_fat);

		memset(tmp_fat, 0, 512);
		for(unsigned a = 1; a <  bpb->sectorsPerFat; a++)
			part->Write(fat_sector + a, (u8*)tmp_fat);

		return Status::Success;
	}

	Status FormatRoot(Block::BlockPartition* part, BPB* bpb, Parameters* p)
	{
		char buffer[p->bytesPerSector];
		memset(buffer, 0, p->bytesPerSector);

		Info info;
		info.bpb = bpb;

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

		BPB bpb;
		memset(&bpb, 0xcc, sizeof(BPB));

		// Init BPB
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

		*(u32*)&bpb.volumeId = 0x6F4C6F59;

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
		part->Read(0, (u8*)bpb);

		Print("Sectors per FAT: %d\n", bpb->sectorsPerFat);

		u32 fatSize = bpb->sectorsPerFat * part->device->drv->BlockSize(part->device->drvPriv);

		Print("Reading FAT... ");
		u8* fat = (u8*)Memory::Alloc(fatSize);
		for(unsigned a = 0; a < bpb->sectorsPerFat; a++)
		{
			part->Read(bpb->reservedSectors /*+ bpb->hiddenSectors*/ + a, fat + (a * 512));
		}
		Print("Done!\n");

		u32 totalSectors = (bpb->totalSectors > 0) ? bpb->totalSectors : bpb->totalSectorsHigh;

		//Info* info = (Info*)Memory::Alloc(sizeof(Info));
		Info* info = new Info();

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

		Print("Unsaved sectors: %d\n", info->cacheObjects.Size());

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
		(*dir) = new Directory;//(bufferSize, 0);
		(*dir)->rootCluster = 0;
		(*dir)->bufferOffset = 0;
		(*dir)->cache = nullptr;

		return Status::Success;
	}

	Status DirectoryClose(void* fs, Directory** dir)
	{
		if(!fs || !dir || !(*dir))
			return Status::Undefined;

		// (*dir)->Flush((Info*)fs);
		Info* info = (Info*)fs;
		FreeCache(info, (*dir)->cache);

		delete (*dir);
		(*dir) = nullptr;

		return Status::Success;
	}

	Status DirectoryReadInternal(Info* info, Directory* dir, DirEntry* entry)
	{
		auto part = info->part;

		//if(dir->dataOffset == 0 || dir->dataOffset == 512 * info->bpb->sectorsPerCluster)
		if(!dir->cache || dir->bufferOffset + sizeof(FAT16_DirEntry) >= dir->cache->bufferSize)
		{
			// dir->Flush(info);

			if(!dir->cache)
			{
				dir->cache = GetCache(info, dir->rootCluster);
			}
			else //if(dir->bufferOffset + sizeof(FAT16_DirEntry) >= dir->cache->bufferSize)
			{
				if(dir->rootCluster == 0)
					//dir->currentCluster++;
					return Status::EndOfFile;
				else
				{
					u32 nextCluster = GetNextCluster(info, dir->cache->cluster);
					if(nextCluster == 0)
						return Status::EndOfFile;

					FreeCache(info, dir->cache);
					dir->cache = GetCache(info, nextCluster);

					//dir->currentCluster = NextCluster(info, dir->currentCluster);
				}

				dir->bufferOffset = 0;
			}

			// Check if eof of root
			// if(dir->firstCluster == 0 && dir->currentCluster >= (info->bpb->rootEntriesCount * 32 / (512 * info->bpb->sectorsPerCluster)))
			// {
			// 	dir->bufferValid = 0;
			// 	return Status::EndOfFile;
			// }
			// Check if eof of non-root
			// if(dir->firstCluster != 0 && dir->currentCluster == 0)
			// {
			// 	dir->bufferValid = 0;
			// 	return Status::EndOfFile;
			// }

			// u32 firstSector = GetSectorToRead(info, dir->currentCluster, IsRoot(dir));
			// // u32 firstSector = (dir->firstCluster == 0)
			// // 		? (info->firstRootSector + dir->currentCluster * info->bpb->sectorsPerCluster)
			// // 		: (info->firstDataSector + (dir->currentCluster - 2) * info->bpb->sectorsPerCluster);

			// for(unsigned a = 0; a < info->bpb->sectorsPerCluster; a++)
			// {
			// 	part->Read(firstSector + a, dir->buffer + 512 * a);
			// }

			// dir->bufferValid = 1;
		}
		else
		{
			// if(dir->cache)
				dir->bufferOffset += 32;
		}

		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->cache->buffer + dir->bufferOffset);

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

		Info* info = (Info*)fs;

		// if(dir->currentCluster != dir->firstCluster) // bufferValid must be zeroed
		// or otherwise, reading directory will skip first entry :(
		{
			// dir->Flush((Info*)fs);
			// dir->currentCluster = dir->firstCluster;
			// dir->bufferValid = 0;
			FreeCache(info, dir->cache);
			dir->cache = nullptr;
		}

		dir->bufferOffset = 0;

		return Status::Success;
	}

	Status DirectoryFollow(void* fs, Directory* dir)
	{
		if(!fs)
			return Status::Undefined;

		// dir->Flush((Info*)fs);

		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->cache->buffer + dir->bufferOffset);

		if(!(fatEntry->attributes & (u8)Attribute::Directory))
		{
			Print("Not a directory!!\n");
			return Status::Undefined;
		}

		Info* info = (Info*)fs;

		dir->rootCluster = fatEntry->cluster;
		dir->bufferOffset = 0;
		FreeCache(info, dir->cache);
		dir->cache = nullptr;

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

			auto currentCluster = dir->rootCluster;
			auto nextCluster = currentCluster;

			do
			{
				currentCluster = nextCluster;
				nextCluster = GetNextCluster(info, currentCluster);
			} while(nextCluster != CLUSTER_LAST);

			AddCluster(info, currentCluster, &nextCluster);

			FreeCache(info, dir->cache);
			dir->cache = GetCache(info, nextCluster);

			// u32 clusterSize = 512 * info->bpb->sectorsPerCluster;
			memset(dir->cache->buffer, 0, dir->cache->bufferSize);
			dir->cache->dirty = true;
			// auto sector = GetSectorToRead(info, nextCluster, false);
			// info->part->Write(sector, (u8*)dir->buffer);

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

		// Print("Adding entry: Cluster: %d, Offset: %d - \"%s\"\n", dir->currentCluster, dir->dataOffset, fatEntryTemplate->name);

		FAT16_DirEntry* newEntry = (FAT16_DirEntry*)(dir->cache->buffer + dir->bufferOffset);
		memcpy(newEntry, fatEntryTemplate, sizeof(*fatEntryTemplate));

		// auto dirLBA = GetSectorToRead(info, dir->currentCluster, IsRoot(dir));
		// info->part->Write(dirLBA, dir->buffer);

		dir->cache->dirty = true;
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
		// dir->Flush(info);

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
		newDir->rootCluster = newDirCluster;
		newDir->bufferOffset = 0;
		newDir->cache = nullptr;

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
		// newDir->Flush(info);
		// ..
		LFNToF83("..", (char*)newEntry.name);
		newEntry.cluster = dir->rootCluster;
		DirectoryAddEntry(info, newDir, &newEntry);

		// newDir->Flush(info);

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
		// dir->Flush(info);

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

		Info* info = (Info*)fs;
		u32 bufferSize = info->part->device->drv->BlockSize(info->part->device->drvPriv) * info->bpb->sectorsPerCluster;
		FAT16_DirEntry* fatEntry = (FAT16_DirEntry*)(dir->cache->buffer + dir->bufferOffset);

		//(*file) = new File(bufferSize, (fatEntry->clusterHigh << 16) | fatEntry->cluster, fatEntry->size, dir->firstCluster);

		(*file) = new File;
		(*file)->rootCluster = fatEntry->cluster;
		(*file)->parentCluster = dir->cache->cluster;
		(*file)->parentOffset = dir->bufferOffset;
		(*file)->totalOffset = 0;
		(*file)->totalSize = fatEntry->size;
		(*file)->cache = nullptr;

		return Status::Success;
	}

	Status FileClose(void* fs, File** file)
	{
		if(!fs)
			return Status::Undefined;

		Print("# Flushing file\n");
		// (*file)->Flush((Info*)fs);

		Info* info = (Info*)fs;

		FreeCache(info, (*file)->cache);

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

		if(file->totalOffset >= file->totalSize)
		{
			Print("EoF\n");
			return Status::EndOfFile;
		}

		//if(!file->bufferValid)
		if(!file->cache || file->totalOffset % file->cache->bufferSize == 0)
		{
			if(file->totalOffset > 0)
			{
				//file->currentCluster = NextCluster(info, file->currentCluster);
				u32 nextCluster = GetNextCluster(info, file->cache->cluster);

				//if(file->currentCluster == 0)
				if(nextCluster == 0)
				{
					Print("Last cluster\n");
					return Status::EndOfFile;
				}

				FreeCache(info, file->cache);
				file->cache = GetCache(info, nextCluster);
			}
			else
			{
				file->cache = GetCache(info, file->rootCluster);
			}

			// u32 sector = info->firstDataSector + (file->currentCluster - 2) * info->bpb->sectorsPerCluster;
			// Print("Reading sector: %d (%d)\n", sector, info->bpb->sectorsPerCluster);
			// part->Read(sector, file->buffer);
			// file->bufferValid = 1;
		}

		// If there's need to read more data than in internal buffer, divide to 2 separate calls
		u32 inBuffer = file->cache->bufferSize - (file->totalOffset % file->cache->bufferSize);
		if(bufferSize > inBuffer)
		{
			Status status;
			u32 subread;

			part->device->drv->Lock(part->device->drvPriv);
			for(unsigned a = 0; a < bufferSize; )
			{
				u32 curSize = (bufferSize - (*readCount) > file->cache->bufferSize ? file->cache->bufferSize : bufferSize - (*readCount));

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

			if(status == Status::EndOfFile && (*readCount) > 0)
				return Status::Success;
			return status;
		}

		// Otherwise, read from internal buffer
		u32 offset = (file->totalOffset % file->cache->bufferSize);
		for(unsigned a = 0; a < bufferSize; a++)
		{
			if(file->totalOffset >= file->totalSize)
				break;

			buffer[a] = file->cache->buffer[offset + a];

			file->totalOffset++;
			(*readCount)++;
		}

		// if(file->totalOffset % file->cache->bufferSize == 0)
		// 	file->bufferValid = 0;

		if((*readCount) == 0)
		{
			Print("_EoF_\n");
			return Status::EndOfFile;
		}

		return Status::Success;
	}

	Status FilePutByte(Info* info, File* file, u8 byte)
	{
		// file->dirty = true;
		Status s;

		// Print("#   put byte: \"%x\" - %d %d\n", byte, file->currentCluster, file->totalDataOffset);

		if(!file->cache)
		{
			if(file->rootCluster == 0)
			{
				u32 nextCluster;
				s = AddCluster(info, 0, &nextCluster);
				if(s != Status::Success)
					return s;

				file->rootCluster = nextCluster;
				file->cache = GetCache(info, file->rootCluster);
				memset(file->cache->buffer, 0, file->cache->bufferSize);

				file->parentNeedsUpdate = true;
			}
			else
			{
				file->cache = GetCache(info, file->rootCluster);
			}
		}
		else if(file->totalOffset > 0 && file->totalOffset % file->cache->bufferSize == 0)
		{
			Print("End of cluster, needs extending...\n");
			// Print("#  ReadNextCluster: %d - %d,%d\n", file->totalDataOffset, file->firstCluster, file->currentCluster);
			u32 nextCluster = GetNextCluster(info, file->cache->cluster);
			if(nextCluster == 0)
			{
				s = AddCluster(info, file->cache->cluster, &nextCluster);
				Print("Adding cluster: %d -> %d\n", file->cache->cluster, nextCluster);
				if(s != Status::Success)
					return s;
			}
			else
				Print("Not extended, cluster is: %d\n", nextCluster);

			FreeCache(info, file->cache);
			file->cache = GetCache(info, nextCluster);
		}

		file->cache->buffer[file->totalOffset % file->cache->bufferSize] = byte;
		file->totalOffset++;

		if(file->totalOffset > file->totalSize)
		{
			file->parentNeedsUpdate = true;
			file->totalSize = file->totalOffset;
		}

		// Update parent~!
		if(file->parentNeedsUpdate)
		{
			Cache* parentCache = GetCache(info, file->parentCluster);
			FAT16_DirEntry* entry = (FAT16_DirEntry*)(parentCache->buffer + file->parentOffset);

			if(!entry->cluster)
				entry->cluster = file->rootCluster;
			entry->size = file->totalSize;

			parentCache->dirty = true;
			FreeCache(info, parentCache);
		}

		return Status::Success;
	}

	Status FileWrite(void* fs, File* file, u8* buffer, u32 bufferSize, u32* writeCount)
	{
		if(!fs || !file)
			return Status::Undefined;

		Status s;
		Print("# Write file: %d\n", bufferSize);

		(*writeCount) = 0;
		for(unsigned a = 0; a < bufferSize; a++)
		{
			s = FilePutByte((Info*)fs, file, buffer[a]);
			if(s != Status::Success)
				return s;

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
