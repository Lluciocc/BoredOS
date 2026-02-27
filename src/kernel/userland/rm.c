#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: rm <path>\n");
        return 1;
    }
    
    // Simple rm (no recursive support yet for simplicity, but can be added)
    if (sys_delete(argv[1]) == 0) {
        printf("Deleted: %s\n", argv[1]);
    } else {
        printf("Error: Cannot delete %s\n", argv[1]);
        return 1;
    }
    return 0;
}
