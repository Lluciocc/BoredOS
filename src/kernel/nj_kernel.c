// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include "memory_manager.h"
#include <stddef.h>

void* njAllocMem(int size) {
    return kmalloc((size_t)size);
}

void njFreeMem(void* block) {
    if (block) kfree(block);
}

void njFillMem(void* block, unsigned char byte, int size) {
    unsigned char *p = (unsigned char*)block;
    for (int i = 0; i < size; i++) {
        p[i] = byte;
    }
}

void njCopyMem(void* dest, const void* src, int size) {
    unsigned char *d = (unsigned char*)dest;
    const unsigned char *s = (const unsigned char*)src;
    for (int i = 0; i < size; i++) {
        d[i] = s[i];
    }
}
