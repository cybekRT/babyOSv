#pragma once

#include"Array.hpp"

namespace Container
{
	class String : protected Array<char>
	{
	public:
		String();
		String(const String& arg);
		String(const char* text);
		~String();

		String operator+(const String& arg) const;
		String& operator=(const String& arg);
		String& operator+=(const String& arg);

		bool operator==(const char* arg) const;
		bool operator==(const String& arg) const;
		bool operator!=(const char* arg) const;
		bool operator!=(const String& arg) const;

		char operator[](u32 index) const;
		void AddAt(u32 index, char c);
		void RemoveAt(u32 index);
		void Clear();

		u32 Length() const;
		const char* Data() const;
	};
}
