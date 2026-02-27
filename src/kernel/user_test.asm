; Copyright (c) 2023-2026 Chris (boreddevnl)
; This software is released under the GNU General Public License v3.0. See LICENSE file for details.
; This header needs to maintain in any file it is present in, as per the GPL license terms.
global user_test_function

section .text
user_test_function:
    ; Syscall convention
.loop:
    ; Invoke SYS_WRITE (Syscall #1)
    mov rdi, 1      ; arg1: fd = 1 (stdout)
    lea rsi, [rel msg] ; arg2: buffer (RIP-relative)
    mov rdx, 15     ; arg3: length
    mov eax, 1      ; syscall_num = 1 (SYS_WRITE)
    syscall

    ; Some delay loop
    mov rcx, 100000000
.delay:
    dec rcx
    jnz .delay

    jmp .loop

msg: db "Hello syscall!", 10
