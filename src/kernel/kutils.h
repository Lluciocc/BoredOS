#ifndef KUTILS_H
#define KUTILS_H

#include <stddef.h>
#include <stdint.h>

// Kernel string utilities
void k_memset(void *dest, int val, size_t len);
void k_memcpy(void *dest, const void *src, size_t len);
size_t k_strlen(const char *str);
int k_strcmp(const char *s1, const char *s2);
void k_strcpy(char *dest, const char *src);
int k_atoi(const char *str);
void k_itoa(int n, char *buf);

// Kernel timing utilities
void k_delay(int iterations);
void k_sleep(int ms);
void k_reboot(void);
void k_shutdown(void);

#endif
