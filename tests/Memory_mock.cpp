#include<cstdlib>

namespace Memory
{
	void* Alloc(unsigned size)
	{
		return malloc(size);
	}

	void Free(void* ptr)
	{
		free(ptr);
	}
}