#include"Terminal.h"
#include<stdarg.h>

namespace Terminal
{
	bool Init()
	{
		unsigned short* data = (unsigned short*)0x800b8000;
		for(unsigned a = 0; a < 80 * 25 * 2; a++) data[a] = 0x0700;
	}

	void PutChar(char c)
	{
		static int p = 0;
		char* vmem = (char*)0x800b8000;

		if(c == '\n')
		{
			p += 160;
			p -= p % 160;
			return;
		}
		else if(c == '\b')
		{
			if(p > 0)
				p-=2;
			vmem[p] = 0;
			return;
		}
		else if(c == '\r')
		{
			p -= p % 160;
			return;
		}
		else if(c == '\t')
		{
			p += 16;
			p -= p % 16;
			return;
		}

		if(p >= 80*24*2)
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

			p -= 160;
		}

		vmem[p + 0] = c;
		vmem[p + 1] = 0x07;
		p+=2;
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
					{
						char tmp[9] = {};
						u32 v = va_arg(args, u32);

						for(u32 a = 0; a < 8; a++)
						{
							tmp[a] = "0123456789ABCDEF"[v >> 28];
							v <<= 4;
						}

						PutString(tmp);

						break;
					}
					case 'u':
					case 'd':
					{
						// TODO edgecase of min-int
						char tmp[11] = {};
						char* buf = tmp;
						s32 v = va_arg(args, s32);

						if(v < 0)
						{
							PutChar('-');
							v = -v;
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
						

						/*for(unsigned a = 0; ; buf--)
						{
							if(a > 10)
								FAIL("wtf");

							char t = tmp[a];
							tmp[a] = *buf;
							*buf = t;

							if(tmp+a < buf)
								break;
						}

						PutString(tmp);*/

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
}
