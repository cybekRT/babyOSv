#include"HAL.hpp"

namespace HAL
{
	u8 In8(u16 port)
	{
		u8 value;

		__asm(
		"mov %1, %%dx \r\n"
		"in %%dx, %%al \r\n"
		"mov %%al, %0 \r\n"
		: "=r"(value) 
		: "r"(port)
		: "eax", "edx");

		return value;
	}

	void Out8(u16 port, u8 data)
	{
		__asm(
		"mov %0, %%dx \r\n"
		"mov %1, %%al \r\n"
		"out %%al, %%dx \r\n"
		: 
		: "r"(port), "r"(data)
		: "eax", "edx");
	}

	u16 In16(u16 port)
	{
		u16 value;

		__asm(
		"mov %1, %%dx \r\n"
		"in %%dx, %%ax \r\n"
		"mov %%ax, %0 \r\n"
		: "=r"(value) 
		: "r"(port)
		: "eax", "edx");

		return value;
	}

	void Out16(u16 port, u16 data)
	{
		__asm(
		"mov %0, %%dx \r\n"
		"mov %1, %%ax \r\n"
		"out %%ax, %%dx \r\n"
		: 
		: "r"(port), "r"(data)
		: "eax", "edx");
	}
}
