int strlen(const char* str)
{
	unsigned len = 0;
	while(*str++)
		len++;

	return len;
}

int strcmp(char* a, char* b)
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

int strcpy(const char* src, char* dst)
{
	int len = 0;
	while(*src)
	{
		*dst++ = *src++;
		len++;
	}

	*dst = 0;
	return len;
}

int strcat(const char* src, char* dst)
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
	return len;
}

// Required if -O3 is used
extern "C"
{
	void* memmove(void* destination, const void* source, unsigned num)
	{
		// FIXME for overlapping buffers
		u8* src = (u8*)source;
		u8* dst = (u8*)destination;
		for(u32 a = 0; a < num; a++)
		{
			*dst++ = *src++;
		}

		return destination;
	}
}

