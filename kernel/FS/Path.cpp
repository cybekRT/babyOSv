#include"Path.hpp"

Path::Path()
{

}

Path::Path(const Container::String& _path)
{
	auto path = _path;

	ASSERT(path[0] == '/' && path.Length() > 2, "Invalid path");

	u32 lastIndex = 1;
	for(unsigned a = 2; a < path.Length(); a++)
	{
		if(path[a] == '/')
		{
			path[a] = 0;
			Add(path.Data() + lastIndex);
			lastIndex = a + 1;
		}
	}

	if(path[path.Length() - 1] != '/')
		Add(path.Data() + lastIndex);
}

Path::~Path()
{

}

void Path::Add(const char* dir)
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
	Container::String str = ToString();
	strcpy(buf, str.Data());
}

Container::String Path::ToString()
{
	Container::String str;

	if(paths.IsEmpty())
	{
		return "/";
	}

	for(auto itr : paths)
	{
		str += "/";
		str += itr;
	}

	return str;
}
