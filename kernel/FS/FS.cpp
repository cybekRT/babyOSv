#include"FS.hpp"

namespace FS
{
	LinkedList<FSInfo*> filesystems;

	bool Init()
	{
		return true;
	}

	void Register(FSInfo* info)
	{
		u8 tmp[32];
		unsigned count = info->Name(tmp);
		tmp[count] = 0;

		Print("Registering filesystem: %s\n", tmp);

		filesystems.PushBack(info);
	}

	void Unregister(FSInfo* info)
	{

	}
}