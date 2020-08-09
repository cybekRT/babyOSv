typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed long long sint64;
typedef signed int sint32;
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

//#define ASSERT(x) ASSERT(x, "")
#define ASSERT(x, y) if(!(x)) { PutString("Assert failed: " y); for(;;){ __asm("cli\nhlt\n"); }; }

#define BREAK {__asm("xchg %%bx, %%bx" : : : "bx");}
#define HALT {for(;;){__asm("cli\nhlt\n");}}

#define ENTER_CRITICAL_SECTION() //{__asm("pushf"); __asm("cli");}
#define EXIT_CRITICAL_SECTION() //{__asm("popf");}

u8 HAL_In(u16 port);
void HAL_Out(u16 port, u8 data);

#include"Terminal.h"