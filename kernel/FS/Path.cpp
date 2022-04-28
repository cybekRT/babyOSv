#include"Path.hpp"

Path::Path()
{

}

Path::Path(const char* pathOrg)
{
	u32 len = strlen(pathOrg);
	char path[len + 1];
	strcpy(path, pathOrg);

	ASSERT(path[0] == '/' && len > 2, "Invalid path");

	u32 lastIndex = 1;
	for(unsigned a = 2; a < len; a++)
	{
		if(path[a] == '/')
		{
			path[a] = 0;
			Add(path + lastIndex);
			lastIndex = a + 1;
		}
	}

	if(path[len - 1] != '/')
		Add(path + lastIndex);
}

Path::~Path()
{

}

void Path::Add(char* dir)
{
	u32 len = strlen(dir);
	//char* tmp = (char*)Memory::Alloc(len + 1);
	char* tmp = new char[len + 1];
	strcpy(tmp, dir);
	paths.PushBack(tmp);
}

void Path::GoUp()
{
	if(!paths.IsEmpty())
	{
		paths.PopBack();
	}
}

void Path::ToString(char* buf)
{
	if(paths.IsEmpty())
	{
		strcpy(buf, "/");
		return;
	}

	buf[0] = 0;
	for(auto itr : paths)
	{
		strcat(buf, "/");
		strcat(buf, itr);
	}
}
