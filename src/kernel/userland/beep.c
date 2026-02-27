#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("BEEP!\n");
    sys_system(14, 1000, 100, 0, 0); // SYSTEM_CMD_BEEP (freq, ms)
    return 0;
}
