#include"HAL.h"

namespace HAL
{
	u8 In(u16 port)
	{
		unsigned char value;

		__asm(
		"mov %1, %%dx \r\n"
		"in %%dx, %%al \r\n"
		"mov %%al, %0 \r\n"
		: "=r"(value) 
		: "r"(port)
		: "eax", "edx");

		return value;
	}

	void Out(u16 port, u8 data)
	{
		__asm(
		"mov %0, %%dx \r\n"
		"mov %1, %%al \r\n"
		"out %%al, %%dx \r\n"
		: 
		: "r"(port), "r"(data)
		: "eax", "edx");
	}
}
