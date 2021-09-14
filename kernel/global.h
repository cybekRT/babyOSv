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

//typedef u32 size_t;

//#define ASSERT(x) ASSERT(x, "")
#define ASSERT(cond, msg) \
{ \
	if(!(cond)) \
	{ \
		Print("\n\nAssert failed: %s:%d (%s) - %s\n\n", __FILE__, __LINE__, __FUNCTION__, msg); \
		for(;;) \
		{ \
			 __asm("int $0xfe\ncli\nhlt\n"); \
		}; \
	} \
}

#define BREAK {__asm("xchg %%bx, %%bx" : : : "bx");}
#define HALT { __asm("hlt"); }
#define FAIL(msg) { Print("\n\nAssert failed: %s:%d (%s) - %s\n\n", __FILE__, __LINE__, __FUNCTION__, msg); __asm("int $0"); }

#include"Terminal.h"

enum class Status
{
	Success = 0,
	Fail = 1,
	Timeout,
};

struct bootloader_info_t;
extern bootloader_info_t* _bootloader_info_ptr;
extern u32 _kernel_beg;
extern u32 _kernel_end;
extern u32 _kernel_code_beg;
extern u32 _kernel_code_end;
extern u32 _kernel_data_beg;
extern u32 _kernel_data_end;
extern u32 _org_stack_beg;
extern u32 _org_stack_end;

int strlen(const char* str);
int strcmp(char* a, char* b);
int strcpy(const char* src, char* dst);
int strcat(const char* src, char* dst);
