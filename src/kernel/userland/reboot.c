#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Rebooting...\n");
    sys_system(12, 0, 0, 0, 0); // SYSTEM_CMD_REBOOT
    return 0;
}
