#include"Path.hpp"

Path::Path()
{
	
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
