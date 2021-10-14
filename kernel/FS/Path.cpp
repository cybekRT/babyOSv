#include"Path.hpp"

Path::Path()
{

}

Path::~Path()
{

}

void Path::Add(char* dir)
{

}

void Path::GoUp()
{
	if(!paths.IsEmpty())
		paths.PopBack();
}
