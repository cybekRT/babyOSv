#pragma once

#include"Container/Array.hpp"
#include"Container/String.hpp"

class Path
{
public:
	Container::Array<Container::String> paths;
public:
	static const u32 MaxLength = 255;

	Path();
	explicit Path(const Container::String& path);
	~Path();

	void Add(const char* dir);
	void GoUp();

	void ToString(char* buf);
	Container::String ToString();
};
