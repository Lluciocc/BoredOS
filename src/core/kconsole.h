#ifndef KCONSOLE_H
#define KCONSOLE_H

#include <stdint.h>
#include <stdbool.h>

void kconsole_init(void);
void kconsole_set_color(uint32_t color);
void kconsole_putc(char c);
void kconsole_write(const char *s);
void kconsole_set_active(bool active);

#endif // KCONSOLE_H
