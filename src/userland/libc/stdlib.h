#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>
#include <stdint.h>

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

#include "string.h"

// Math/Utility functions
int atoi(const char *nptr);
void itoa(int n, char *buf);

// IO functions
void puts(const char *s);
void printf(const char *fmt, ...);

// System/Process functions
int chdir(const char *path);
char* getcwd(char *buf, int size);
void sleep(int ms);
void exit(int status);

#endif
