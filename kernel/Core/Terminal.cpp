#include"Terminal.hpp"
#include<stdarg.h>

namespace Terminal
{
	char* buffer = nullptr;
	u32 cursorX = 0;
	u32 cursorY = 0;
	u8 color = 0x07;

	bool Init()
	{
		unsigned short* data = (unsigned short*)0x800b8000;
		for(unsigned a = 0; a < 80 * 25; a++)
			data[a] = 0x0000;

		buffer = nullptr;
		cursorX = 0;
		cursorY = 0;
		color = 0x07;

		return true;
	}

	u32 CursorToBuffer()
	{
		return cursorY * 160 + cursorX * 2;
	}

	u8* GetBuffer()
	{
		return (u8*)buffer;
	}

	void SetBuffer(u8* buffer)
	{
		Terminal::buffer = (char*)buffer;
	}

	u32 PutChar(char c)
	{
		if(buffer)
		{
			(*buffer++) = c;
			return 1;
		}

		char* const vmem = (char*)0x800b8000;

		bool handled = false;
		if(c == '\n')
		{
			cursorY++;
			cursorX = 0;
			
			handled = true;
		}
		else if(c == '\b')
		{
			if(cursorX > 0)
				cursorX--;
			else if(cursorY > 0)
			{
				cursorY--;
				cursorX = 79;
			}

			vmem[CursorToBuffer()] = 0;
			return 0;
		}
		else if(c == '\r')
		{
			cursorX = 0;
			return 0;
		}
		else if(c == '\t')
		{
			cursorX += 8;
			cursorX -= cursorX % 8;

			handled = true;
		}

		if(cursorY > 24)
		{
			unsigned short* src = (unsigned short*)0x800b80a0;
			unsigned short* dst = (unsigned short*)0x800b8000;

			for(u32 a = 0; a < 80*24; a++)
			{
				*dst++ = *src++;
			}

			for(u32 a = 0; a < 80; a++)
			{
				*dst++ = 0x0700;
			}

			cursorY--;
		}

		if(handled == true)
			return 0;

		auto bufPos = CursorToBuffer();
		vmem[bufPos + 0] = c;
		vmem[bufPos + 1] = color;

		cursorX++;
		if(cursorX >= 80)
		{
			cursorX = 0;
			cursorY++;
		}

		return 1;
	}

	u32 PutString(const char* s)
	{
		u32 len = 0;

		while(*s)
		{
			len += PutChar(*s);
			s++;
		}

		return len;
	}

	static u32 PrintInternal(const char *fmt, va_list args)
	{
		__asm("pushf\ncli");
		u32 len = 0;

		bool isFormat = false;
		while(*fmt)
		{
			char c = *fmt;

			if(c == '%')
			{
				if(!isFormat)
				{
					isFormat = true;
				}
				else
				{
					len += PutChar(c);
				}
				
			}
			else if(isFormat)
			{
				isFormat = false;

				switch(c)
				{
					case 'c':
					{
						char v = va_arg(args, u32);
						len += PutChar(v);

						break;
					}
					case 'p':
					case 'x':
					case 'X':
					{
						char tmp[9] = {};
						u32 v = va_arg(args, u32);

						for(u32 a = 0; a < 8; a++)
						{
							tmp[a] = "0123456789ABCDEF"[v >> 28];
							v <<= 4;
						}

						if(c == 'X')
							len += PutString(tmp + 6);
						else
							len += PutString(tmp);

						break;
					}
					case 'u':
					case 'd':
					{
						// TODO edgecase of min-int
						char tmp[11] = {};
						char* buf = tmp;
						u32 v = va_arg(args, u32);

						if(c == 'd' && (s32)v < 0)
						{
							len += PutChar('-');
							v = -(s32)v;
						}

						do
						{
							*buf++ = "0123456789"[v % 10];
							v /= 10;
						} while(v > 0);
						
						buf--;

						do
						{
							len += PutChar(*buf--);
						} while(buf >= tmp);

						break;
					}
					case 's':
					{
						const char* v = va_arg(args, const char*);
						len += PutString(v);

						break;
					}
					default:
					{
						len += PutChar('%');
						len += PutChar(c);
						break;
					}
				}
			}
			else
			{
				len += PutChar(*fmt);
			}
			
			fmt++;
		}

		__asm("popf");
		return len;
	}

	u32 Print(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		u32 len = PrintInternal(fmt, args);
		va_end(args);

		return len;
	}

	u32 PrintWithPadding(u32 pad, const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		u32 len = PrintInternal(fmt, args);
		va_end(args);

		for(unsigned a = len; a < pad; a++)
			PutChar(' ');

		return len;
	}

	u32 PutHex(unsigned long v)
	{
		return Print("%x", v);
	}

	void GetXY(u32* x, u32* y)
	{
		*x = cursorX;
		*y = cursorY;
	}

	void SetXY(u32 x, u32 y)
	{
		cursorX = x;
		cursorY = y;
	}

	void GetColor(u8* fg, u8* bg)
	{
		*bg = (color & 0xF0) >> 4;
		*fg = (color & 0x0F);
	}

	void SetColor(u8 fg, u8 bg)
	{
		color = (bg & 0x0F) << 4 | (fg & 0x0F);
	}
}
