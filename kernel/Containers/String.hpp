#pragma once

class String
{
protected:
	char buffer_s[32];
	char* buffer;
	u32 length;
	u32 capacity;

public:
	String() : buffer(nullptr), length(0)
	{
		buffer_s[0] = 0;
	}

	explicit String(const char* arg) : String()
	{
		// auto argLen = strlen(arg);
		// if(argLen < sizeof(buffer_s) + 1)
		// {
		// 	strcpy(buffer_s, arg);
		// }
		// else
		// {
		// 	Realloc(argLen + 1);
		// 	strcpy(buffer, arg);
		// }

		// length = argLen;

		Print("Construct String(%p) -> %p\n", arg, this);
		Assign(arg, strlen(arg));
	}

	String(const String& arg) : String()
	{
		// if(arg.buffer)
		// {
		// 	Realloc(arg.capacity);
		// 	strcpy(buffer, arg.buffer);
		// 	length = arg.length;
		// }
		// else
		// {
		// 	strcpy(buffer_s, arg.buffer_s);
		// 	length = arg.length;
		// }

		if(arg.buffer)
			Assign(arg.buffer, arg.length);
		else
			Assign(arg.buffer_s, arg.length);
	}

	~String()
	{
		if(buffer)
			delete[] buffer;
	}

	String& operator=(const char* arg)
	{
		Assign(arg, strlen(arg));

		return *this;
	}

	String& operator=(const String& arg)
	{
		if(arg.buffer)
			Assign(arg.buffer, arg.length);
		else
			Assign(arg.buffer_s, arg.length);

		return *this;
	}

	char& operator[](u32 index)
	{
		if(!buffer)
		{
			ASSERT(index < sizeof(buffer_s), "invalid index");
			return buffer_s[index];
		}
		else
		{
			ASSERT(index < length, "invalid index");
			return buffer[index];
		}
	}

	String operator+(const char* arg) const
	{
		String s(*this);
		s.Add(arg, strlen(arg));

		return s;
	}

	String operator+(const String& arg) const
	{
		String s(*this);
		if(arg.buffer)
			s.Add(arg.buffer, arg.length);
		else
			s.Add(arg.buffer_s, arg.length);

		return s;
	}

	String& operator+=(const char* arg)
	{
		Add(arg, strlen(arg));

		return *this;
	}

	String& operator+=(const String& arg)
	{
		if(arg.buffer)
			Add(arg.buffer, arg.length);
		else
			Add(arg.buffer_s, arg.length);

		return *this;
	}

	char* Data()
	{
		if(buffer)
			return buffer;
		else
			return buffer_s;
	}

	u32 Length()
	{
		return length;
	}

	u32 Capacity()
	{
		if(!buffer)
			return sizeof(buffer_s) - 1;
		else
			return capacity;
	}

protected:
	bool Realloc(u32 minReqSize = 0)
	{
		Print("Resizing!\n");
		if(!buffer)
		{
			Print("No buffer:\n");
			capacity = sizeof(buffer_s) * 2;
			if(capacity <= minReqSize)
				capacity = minReqSize + 1;

			buffer = new char[capacity];
			if(!buffer)
				return false;

			if(length > 0)
				strcpy(buffer, buffer_s);
			else
				buffer[0] = 0;
		}
		else
		{
			Print("Yes buffer:\n");
			capacity *= 2;
			if(capacity <= minReqSize)
				capacity = minReqSize + 1;

			char* newData = new char[capacity];
			if(!newData)
				return false;

			if(length > 0)
				strcpy(newData, buffer);
			else
				newData[0] = 0;

			delete[] buffer;
			buffer = newData;
		}

		return true;
	}

	void Assign(const char* arg, u32 argLen)
	{
		Print("String assign: %s (%p)\n", arg, this);
		if(buffer)
		{
			if(capacity > argLen)
			{
				Print("Enough capacity\n");
				strcpy(buffer, arg);
			}
			else
			{
				Print("Resize needed... %p\n", buffer);
				delete[] buffer;
				buffer = nullptr;

				Print("Removed, reallocating...\n");
				length = 0;
				Realloc(argLen + 1);
				Print("Reallocated, copying string\n");
				strcpy(buffer, arg);
			}
		}
		else
		{
			if(argLen < sizeof(buffer_s) + 1)
			{
				strcpy(buffer_s, arg);
			}
			else
			{
				length = 0;
				Realloc(argLen + 1);
				strcpy(buffer, arg);
			}
		}

		length = argLen;
		Print("Assigned~! (%p)\n", this);
	}

	void Add(const char* arg, u32 argLen)
	{
		char* dst;
		u32 dstLen = length + argLen;

		if(!buffer && dstLen + 1 <= sizeof(buffer_s))
			dst = buffer_s;
		else
		{
			if(capacity > dstLen + 1)
				dst = buffer;
			else
			{
				Realloc(dstLen + 1);
				dst = buffer;
			}
		}

		strcat(dst, arg);
	}
};
