#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Shutting down...\n");
    sys_system(13, 0, 0, 0, 0); // SYSTEM_CMD_SHUTDOWN
    return 0;
}
