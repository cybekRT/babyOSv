#pragma once

#include"HAL.hpp"

#define RGB(r, g, b) (( r & 0b11100000 ) | (( g & 0b11000000 ) >> 3) | (( b & 0b11100000 ) >> 5))

namespace VGA
{
	u8 Read_3C0(u8 index);
	void Write_3C0(u8 index, u8 value);

	u8 Read_3C2();
	void Write_3C2(u8 value);

	u8 Read_3C4(u8 index);
	void Write_3C4(u8 index, u8 value);

	void Read_3C8(u8 index, u8* r, u8* g, u8* b);
	void Write_3C8(u8 index, u8 r, u8 g, u8 b);

	u8 Read_3CE(u8 index);
	void Write_3CE(u8 index, u8 value);

	u8 Read_3D4(u8 index);
	void Write_3D4(u8 index, u8 value);

	bool Init();

	void SetCursor(bool enabled);
}