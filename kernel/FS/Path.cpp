#include"Path.hpp"

Path::Path()
{
	Print("%s:%d - %s\n", __FILE__, __LINE__, __FUNCTION__);
}

Path::~Path()
{

}

void Path::Add(char* dir)
{
	u32 len = strlen(dir);
	char* tmp = (char*)Memory::Alloc(len + 1);
	strcpy(dir, tmp);
	paths.PushBack(tmp);
}

void Path::GoUp()
{
	if(!paths.IsEmpty())
		paths.PopBack();
}

void Path::ToString(char* buf)
{
	if(paths.IsEmpty())
	{
		strcpy("/", buf);
		return;
	}

	Print("This: %p\n", this);
	Print("This: %p\n", &paths);
	Print("Path parts: %d\n", paths.Size());
	buf[0] = 0;
	for(auto itr : paths)
	{
		strcat("/", buf);
		strcat(itr.value, buf);
	}
}
