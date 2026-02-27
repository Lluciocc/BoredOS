; Copyright (c) 2023-2026 Chris (boreddevnl)
; This software is released under the GNU General Public License v3.0. See LICENSE file for details.
; This header needs to maintain in any file it is present in, as per the GPL license terms.
global test_syscall
section .text

test_syscall:
    ; syscall number in RDI
    mov rdi, 1
    ; string pointer in RSI
    lea rsi, [rel test_msg]
    
    ; The SYSCALL instruction
    syscall
    
    ret

section .rodata
test_msg: db "Hello from Syscall!", 10, 0
