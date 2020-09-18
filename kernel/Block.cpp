#include"Block.hpp"
#include"LinkedList.h"

namespace Block
{
	LinkedList<BlockInfo*> devices;

	bool Init()
	{
		return true;
	}

	void Register(BlockInfo* info)
	{
		u8 tmp[128];
		u8 count = info->Name(info->dev, tmp);
		tmp[count] = 0;

		Print("Registering device: %s\n", tmp);
		devices.PushBack(info);
	}

	void Unregister(BlockInfo* info)
	{

	}
}
