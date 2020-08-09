#pragma once

namespace Terminal
{
	bool Init();

	void PutChar(char c);
	void PutString(const char* s);
	void Print(const char *fmt, ...);
	void PutHex(unsigned long v);
}

using Terminal::PutHex;
using Terminal::PutString;
using Terminal::Print;
