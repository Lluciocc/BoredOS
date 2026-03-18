#!/bin/bash
sed -i '' -e 's/asm volatile("pushfq; pop %0; cli" : "=r"(rflags));/rflags = wm_lock_acquire();/g' src/sys/syscall.c
sed -i '' -e 's/asm volatile("push %0; popfq" : : "r"(rflags));/wm_lock_release(rflags);/g' src/sys/syscall.c
sed -i '' -e 's/asm volatile("pushfq; pop %0; cli" : "=r"(rflags));/rflags = wm_lock_acquire();/g' src/wm/wm.c
sed -i '' -e 's/asm volatile("push %0; popfq" : : "r"(rflags));/wm_lock_release(rflags);/g' src/wm/wm.c
sed -i '' -e 's/uint64_t rflags;/uint64_t rflags;/g' src/sys/syscall.c
echo "Done"
