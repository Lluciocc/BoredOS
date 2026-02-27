#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cat <filename>\n");
        return 1;
    }
    
    int fd = sys_open(argv[1], "r");
    if (fd < 0) {
        printf("Error: Cannot open %s\n", argv[1]);
        return 1;
    }
    
    char buffer[4096];
    int bytes;
    while ((bytes = sys_read(fd, buffer, sizeof(buffer))) > 0) {
        sys_write(1, buffer, bytes);
    }
    
    sys_close(fd);
    return 0;
}
