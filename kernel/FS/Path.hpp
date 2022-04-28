#pragma once

#include"Container/LinkedList.hpp"

class Path
{
public:
	Container::LinkedList<char*> paths;
public:
	static const u32 MaxLength = 255;

	Path();
	explicit Path(const char* path);
	~Path();

	void Add(char* dir);
	void GoUp();

	void ToString(char* buf);
};
