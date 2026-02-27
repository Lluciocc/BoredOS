#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("What manual page do you want?\nExample: man ls\n");
        return 0;
    }
    
    char path[128];
    printf("Manual for: %s\n", argv[1]);
    printf("---------------------------\n");
    
    strcpy(path, "A:/Library/man/");
    strcat(path, argv[1]);
    strcat(path, ".txt");
    
    int fd = sys_open(path, "r");
    if (fd < 0) {
        printf("No manual entry for %s\n", argv[1]);
        return 1;
    }
    
    char buffer[4096];
    int bytes;
    while ((bytes = sys_read(fd, buffer, sizeof(buffer))) > 0) {
        sys_write(1, buffer, bytes);
    }
    
    sys_close(fd);
    printf("\n");
    return 0;
}
