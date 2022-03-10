#include<cstdlib>
#include<cstdio>
#include<stdarg.h>

namespace Terminal
{
	u32 Print(const char* fmt, ...)
	{
		u32 len;

		va_list args;
		va_start(args, fmt);
		len = vprintf(fmt, args);
		va_end(args);

		return len;
	}
}