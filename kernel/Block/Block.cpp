#include"Block.hpp"
#include"LinkedList.h"

int strcpy(const char* src, char* dst);

namespace Block
{
	Array<BlockDevice> devices;
	uint8 devicesTypesCount[(unsigned)Type::Count] = { 0 };
	const char* typeName[] = { "unk", "fdd", "hdd", "cd" };

	bool Init()
	{
		return true;
	}

	void Register(Type type, BlockDriver* drv, void* dev)
	{
		BlockDevice* bd = (BlockDevice*)Memory::Malloc(sizeof(BlockDevice));
		bd->type = type;
		bd->drv = drv;
		bd->dev = drv;

		auto nameLen = strcpy(typeName[(unsigned)type], (char*)bd->name);
		bd->name[nameLen] = '0' + devicesTypesCount[(unsigned)type]++;
		bd->name[nameLen+1] = 0;

		Print("Registering device: %s\n", bd->name);
		devices.PushBack(*bd);
	}

	void Unregister(Type type, BlockDriver* drv, void* dev)
	{

	}
}
