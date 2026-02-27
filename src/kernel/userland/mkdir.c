#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: mkdir <dirname>\n");
        return 1;
    }
    
    if (sys_mkdir(argv[1]) == 0) {
        printf("Created directory: %s\n", argv[1]);
    } else {
        printf("Error: Cannot create directory %s\n", argv[1]);
        return 1;
    }
    return 0;
}
