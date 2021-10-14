#include"Path.hpp"

Path::Path()
{

}

Path::~Path()
{

}

void Path::Add(char* dir)
{
	paths.PushBack(dir);
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

	buf[0] = 0;
	for(auto itr : paths)
	{
		strcat("/", buf);
		strcat(itr.value, buf);
	}
}
