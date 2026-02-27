#include <stdlib.h>
#include <syscall.h>

typedef struct {
    uint16_t vendor;
    uint16_t device;
    uint8_t class_code;
    uint8_t subclass;
} pci_info_t;

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    int count = sys_system(17, 0, 0, 0, 0); // Get count
    if (count < 0) {
        printf("Error: Could not retrieve PCI device count.\n");
        return 1;
    }
    
    printf("PCI Devices (%d found):\n", count);
    printf("---------------------------\n");
    for (int i = 0; i < count; i++) {
        pci_info_t info;
        if (sys_system(17, (uint64_t)&info, i, 0, 0) == 0) {
            printf("[%d] Vendor:%04x Device:%04x Class:%02x Sub:%02x\n", 
                   i, info.vendor, info.device, info.class_code, info.subclass);
        }
    }
    
    return 0;
}
