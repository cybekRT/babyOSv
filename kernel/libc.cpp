extern "C"
{

void __cxa_pure_virtual()
{
	ASSERT(false, "Pure virtual function called :(");
}

size_t strlen(const char* str)
{
	unsigned len = 0;
	while(*str++)
		len++;

	return len;
}

int strcmp(const char* a, const char* b)
{
	while(*a || *b)
	{
		if(*a != *b)
			return (*a - *b);

		a++;
		b++;
	}

	return 0;
}

char* strcpy(char* dst, const char* src)
{
	int len = 0;
	while(*src)
	{
		*dst++ = *src++;
		len++;
	}

	*dst = 0;
	return dst;
}

char* strcat(char* dst, const char* src)
{
	int len = 0;

	while(*dst)
	{
		dst++;
	}

	while(*src)
	{
		*dst++ = *src++;
		len++;
	}

	*dst = 0;
	return dst;
}

// Required if -O3 is used
void* memmove(void* dst, const void* src, size_t len)
{
	// FIXME for overlapping buffers
	u8* _src = (u8*)src;
	u8* _dst = (u8*)dst;
	for(u32 a = 0; a < len; a++)
	{
		*_dst++ = *_src++;
	}

	return dst;
}

void* memset(void* ptr, int c, size_t len)
{
	unsigned char* p = (unsigned char*)ptr;
	for(unsigned a = 0; a < len; a++)
	{
		p[a] = c;
	}

	return ptr;
}

void* memcpy(void* dst, const void* src, size_t len)
{
	u8* _src = (u8*)src;
	u8* _dst = (u8*)dst;

	for(unsigned a = 0; a < len; a++)
		_dst[a] = _src[a];

	return dst;
}

int tolower(int c)
{
	if(c >= 'A' && c <= 'Z')
	{
		return c - ('A' - 'a');
	}
	else
	{
		return c;
	}
}

int toupper(int c)
{
	if(c >= 'a' && c <= 'z')
	{
		return c - ('a' - 'A');
	}
	else
	{
		return c;
	}
}

}
