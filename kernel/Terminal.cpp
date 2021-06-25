#include"Terminal.h"
#include<stdarg.h>

namespace Terminal
{
	u32 cursorX = 0;
	u32 cursorY = 0;
	u8 color = 0x07;

	bool Init()
	{
		unsigned short* data = (unsigned short*)0x800b8000;
		for(unsigned a = 0; a < 80 * 25 * 2; a++) data[a] = 0x0700;
	}

	u32 CursorToBuffer()
	{
		return cursorY * 160 + cursorX * 2;
	}

	void PutChar(char c)
	{
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
			return;
		}
		else if(c == '\r')
		{
			cursorX = 0;
			return;
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

			for(unsigned a = 0; a < 80*24; a++)
			{
				*dst++ = *src++;
			}

			for(unsigned a = 0; a < 80; a++)
			{
				*dst++ = 0x0700;
			}

			cursorY--;
		}

		if(handled == true)
			return;

		auto bufPos = CursorToBuffer();
		vmem[bufPos + 0] = c;
		vmem[bufPos + 1] = color;

		cursorX++;
		if(cursorX >= 80)
		{
			cursorX = 0;
			cursorY++;
		}
	}

	void PutString(const char* s)
	{
		while(*s)
		{
			PutChar(*s);
			s++;
		}
	}

	void Print(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

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
					PutChar(c);
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
						PutChar(v);

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
							PutString(tmp + 6);
						else
							PutString(tmp);

						break;
					}
					case 'u':
					case 'd':
					{
						// TODO edgecase of min-int
						char tmp[11] = {};
						char* buf = tmp;
						u32 v = va_arg(args, u32);

						if(c == 'd' && v < 0)
						{
							PutChar('-');
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
							PutChar(*buf--);
						} while(buf >= tmp);

						break;
					}
					case 's':
					{
						const char* v = va_arg(args, const char*);
						PutString(v);

						break;
					}
					default:
					{
						PutChar('%');
						PutChar(c);
						break;
					}
				}
			}
			else
			{
				PutChar(*fmt);
			}
			
			fmt++;
		}

		va_end(args);
	}

	void PutHex(unsigned long v)
	{
		Print("%x", v);
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
