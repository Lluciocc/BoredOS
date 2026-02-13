#include "cli_utils.h"
#include "io.h"

struct idtr_t {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void cli_cmd_reboot(char *args) {
    (void)args;
    cli_write("Rebooting...\n");
    

    for (int i = 0; i < 100; i++) {
        if ((inb(0x64) & 1) != 0) (void)inb(0x60);
        cli_delay(10000);
    }
    
    // Pulse reset line
    for (int i = 0; i < 100; i++) {
        uint8_t temp = inb(0x64);
        if ((temp & 2) == 0) break;
        cli_delay(10000);
    }
    outb(0x64, 0xFE);
    cli_delay(5000000); // Wait for reset
    

    struct idtr_t idtr_invalid = { 0, 0 };
    
    asm volatile ("cli");
    asm volatile ("lidt %0" : : "m"(idtr_invalid));
    asm volatile ("int $3"); 
    
    while(1) {
        asm volatile ("hlt");
    }
}
