#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    sys_system(14, 392, 400, 0, 0);
    printf("BEEP!\n");
    return 0;
}
