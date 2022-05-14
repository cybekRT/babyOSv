#pragma once

#include"Containers/Array.hpp"
#include"Containers/String.hpp"

class Path
{
public:
	Array<String> paths;
public:
	static const u32 MaxLength = 255;

	Path();
	Path(const Path& arg);
	explicit Path(const String& path);
	~Path();

	Path& operator=(const Path& arg);

	void Add(const char* dir);
	void GoUp();

	void ToString(char* buf);
	String ToString();
};
