#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    sys_system(10, 0, 0, 0, 0); // SYSTEM_CMD_CLEAR_SCREEN
    return 0;
}
