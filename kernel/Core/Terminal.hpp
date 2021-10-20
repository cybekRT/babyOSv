#pragma once

namespace Terminal
{
	bool Init();

	void SetBuffer(u8* buffer);

	u32 PutChar(char c);
	u32 PutString(const char* s);
	u32 Print(const char *fmt, ...);
	u32 PrintWithPadding(u32 pad, const char *fmt, ...);
	u32 PutHex(unsigned long v);

	void GetXY(u32* x, u32* y);
	void SetXY(u32 x, u32 y);
	void GetColor(u8* fg, u8* bg);
	void SetColor(u8 fg, u8 bg);
}

using Terminal::PutHex;
using Terminal::PutString;
using Terminal::Print;
