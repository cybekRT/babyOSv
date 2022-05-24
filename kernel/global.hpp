typedef unsigned long long uint64;
typedef unsigned long int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed long long sint64;
typedef signed long int sint32;
typedef signed char sint16;
typedef signed char sint8;

typedef uint64 u64;
typedef uint32 u32;
typedef uint16 u16;
typedef uint8 u8;

typedef sint64 s64;
typedef sint32 s32;
typedef sint16 s16;
typedef sint8 s8;

#ifdef TESTS
#include<cstddef>
#else
typedef u32 size_t;
#endif

#ifndef ASSERT
#define ASSERT(cond, msg) \
{ \
	if(!(cond)) \
	{ \
		Terminal::Print("\nAssert failed: %s:%d (%s) - %s\n", __FILE__, __LINE__, __FUNCTION__, msg); \
		for(;;) \
		{ \
			 __asm("int $0xfe\ncli\nhlt\n"); \
		}; \
	} \
}
#endif

#ifndef TESTS

#define DEBUG_BREAK {__asm("xchg %%bx, %%bx" : : : "bx");}
#define FAIL(msg) { Print("\nAssert failed: %s:%d (%s) - %s\n", __FILE__, __LINE__, __FUNCTION__, msg); __asm("int $0xfe"); }

#endif

#include"Terminal.hpp"

#ifndef TESTS
#include"libc.hpp"
#endif
