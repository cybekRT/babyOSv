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

// Memory
// void* operator new(size_t size);
// void* operator new[](size_t size);
// void operator delete(void* ptr, size_t size);
// void operator delete(void* ptr);
// void operator delete[](void* ptr, size_t size);
// void operator delete[](void* ptr);
}