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
	Path(const Path& arg);
	explicit Path(const Container::String& path);
	~Path();

	Path& operator=(const Path& arg);

	void Add(const char* dir);
	void GoUp();

	void ToString(char* buf);
	Container::String ToString();
};
