#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    int dt[6];
    if (sys_system(11, (uint64_t)dt, 0, 0, 0) == 0) {
        printf("Current Date: %d-%d-%d %d:%d:%d\n", dt[0], dt[1], dt[2], dt[3], dt[4], dt[5]);
    } else {
        printf("Error: Could not retrieve date.\n");
    }
    return 0;
}
