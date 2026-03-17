// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#ifndef SMP_H
#define SMP_H

#include <stdint.h>
#include <stdbool.h>
#include "spinlock.h"

// Per-CPU state. Dynamically allocated at boot based on actual CPU count.
typedef struct cpu_state {
    uint32_t cpu_id;           // Logical CPU index (0 = BSP)
    uint32_t lapic_id;         // Local APIC ID from Limine
    uint64_t kernel_stack;     // Top of kernel stack for this CPU
    void    *kernel_stack_alloc; // Base allocation for kfree
    volatile bool online;      // True once AP is fully initialized
} cpu_state_t;

// Initialize SMP — call after GDT/IDT/memory init but before wm_init.
// Pass the Limine SMP response. APs will be started and will enter their
// idle loops. Returns the number of CPUs brought online.
struct limine_smp_response;
uint32_t smp_init(struct limine_smp_response *smp_resp);

// Get the current CPU index (0 = BSP). Uses CPUID to read LAPIC ID,
// then looks up in the cpu table.
uint32_t smp_this_cpu_id(void);

// Total number of CPUs online.
uint32_t smp_cpu_count(void);

// Get per-CPU state by index.
cpu_state_t *smp_get_cpu(uint32_t cpu_id);

#endif
