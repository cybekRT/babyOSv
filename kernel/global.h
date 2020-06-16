typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef signed int sint32;
typedef signed char sint16;
typedef signed char sint8;

typedef uint32 u32;
typedef uint16 u16;
typedef uint8 u8;
typedef sint32 s32;
typedef sint16 s16;
typedef sint8 s8;

//#define ASSERT(x) ASSERT(x, "")
#define ASSERT(x, y) if(!(x)) { void PutString(const char *); PutString("Assert failed: " y); for(;;){ __asm("cli\nhlt\n"); }; }
