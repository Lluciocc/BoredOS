#ifndef BOREDOS_LIBC_STRING_H
#define BOREDOS_LIBC_STRING_H

#include <stddef.h>

void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
size_t strlen(const char *s);
int strcmp(const char *s1, const char *s2);
char* strcpy(char *dest, const char *src);
char* strcat(char *dest, const char *src);

#endif
