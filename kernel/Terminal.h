#pragma once

namespace Terminal
{
	bool Init();

	void PutChar(char c);
	void PutString(const char* s);
	void Print(const char *fmt, ...);
	void PutHex(unsigned long v);

	void GetXY(u32* x, u32* y);
	void SetXY(u32 x, u32 y);
	void GetColor(u8* fg, u8* bg);
	void SetColor(u8 fg, u8 bg);
}

using Terminal::PutHex;
using Terminal::PutString;
using Terminal::Print;
