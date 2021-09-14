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
	{
		Print("Popping path~! Before: %d~!", paths.Size());
		paths.PopBack();
		Print("New size: %d\n", paths.Size());
	}
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
		Print("Itr: \"%s\"\n", itr);
		//Print("- %p (%x) - %p\n", itr, (*itr), buf);
		strcat("/", buf);
		Print("- \"%s\"\n", buf);
		//strcat((const char*)*itr, buf);
		strcat(itr, buf);
		Print("- \"%s\"\n", buf);
	}
}
