#pragma once

extern "C"
{
// Strings
size_t strlen(const char* str);
int strcmp(const char* a, const char* b);
char* strcpy(char* dst, const char* src);
char* strcat(char* dst, const char* src);

void* memmove(void* dst, const void* src, size_t len);
void* memset(void* ptr, int c, size_t len);
void* memcpy(void* dst, const void* src, size_t len);

int tolower(int c);
int toupper(int c);

inline void *operator new(size_t, void *p)     throw() { return p; }
inline void *operator new[](size_t, void *p)   throw() { return p; }
inline void  operator delete  (void *, void *) throw() { };
inline void  operator delete[](void *, void *) throw() { };

// Memory
// void* operator new(size_t size);
// void* operator new[](size_t size);
// void operator delete(void* ptr, size_t size);
// void operator delete(void* ptr);
// void operator delete[](void* ptr, size_t size);
// void operator delete[](void* ptr);
}