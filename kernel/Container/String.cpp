#include"String.hpp"

namespace Container
{
	String::String() : Array(8)
	{
		PushBack(0);
	}

	String::String(const String& arg) : Array(arg)
	{

	}

	String::String(const char* text) : Array(strlen(text) + 1)
	{
		u32 len = strlen(text);
		for(unsigned a = 0; a <= len; a++)
			PushBack(text[a]);
	}

	String::~String()
	{
		Print("~String - %p\n", this);
		Clear();
	}

	String& String::operator=(const String& arg)
	{
		Array::operator=(arg);
		return *this;
	}

	String String::operator+(char arg) const
	{
		char tmp[2] = { arg, 0 };
		return *this + String(tmp);
	}

	String String::operator+(const char* arg) const
	{
		return *this + String(arg);
	}

	String String::operator+(const String& arg) const
	{
		u32 len = Length() + arg.Length();
		char* tmp = new char[len + 1];

		unsigned a, b;
		for(a = 0; a < Length(); a++)
			tmp[a] = objs[a];
		for(b = 0; b < arg.Length(); b++)
			tmp[a + b] = arg[b];

		tmp[a + b] = 0;
		String ret(tmp);
		delete[] tmp;

		return ret;
	}

	String& String::operator+=(char arg)
	{
		objs[size - 1] = arg;
		PushBack(0);
	}

	String& String::operator+=(const char* arg)
	{
		return *this += String(arg);
	}

	String& String::operator+=(const String& arg)
	{
		PopBack();
		for(unsigned a = 0; a < arg.Length(); a++)
			PushBack(arg[a]);
		PushBack(0);
	}

	bool String::operator==(const char* arg) const
	{
		for(unsigned a = 0; a < Length() + 1; a++)
		{
			if(arg[a] != objs[a])
				return false;
		}

		return true;
	}

	bool String::operator==(const String& arg) const
	{
		if(Length() != arg.Length())
			return false;

		for(unsigned a = 0; a < Length() + 1; a++)
		{
			if(arg.objs[a] != objs[a])
				return false;
		}

		return true;
	}

	bool String::operator!=(const char* arg) const
	{
		for(unsigned a = 0; a < Length() + 1; a++)
		{
			if(arg[a] != objs[a])
				return true;
		}

		return false;
	}

	bool String::operator!=(const String& arg) const
	{
		if(Length() != arg.Length())
			return true;

		for(unsigned a = 0; a < Length() + 1; a++)
		{
			if(arg.objs[a] != objs[a])
				return true;
		}

		return false;
	}

	char String::operator[](u32 index) const
	{
		return Array::operator[](index);
	}

	char& String::operator[](u32 index)
	{
		return Array::operator[](index);
	}

	void String::AddAt(u32 index, char c)
	{
		Array::Insert(Array::begin() + index, c);
	}

	void String::RemoveAt(u32 index)
	{
		Array::RemoveAt(index);
	}

	void String::Clear()
	{
		Array::Clear();
		Array::PushBack(0);
	}

	u32 String::Length() const
	{
		return size - 1;
	}

	const char* String::Data() const
	{
		return objs;
	}
}
