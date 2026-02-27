#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: touch <filename>\n");
        return 1;
    }
    
    // Check if file already exists
    if (sys_exists(argv[1])) {
        // Just return success if it exists (simplification)
        return 0;
    }
    
    int fd = sys_open(argv[1], "w");
    if (fd < 0) {
        printf("Error: Cannot create %s\n", argv[1]);
        return 1;
    }
    
    sys_close(fd);
    return 0;
}
