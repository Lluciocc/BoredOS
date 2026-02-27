// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: mv <source> <dest>\n");
        return 1;
    }
    
    int fd_in = sys_open(argv[1], "r");
    if (fd_in < 0) {
        printf("Error: Cannot open source %s\n", argv[1]);
        return 1;
    }
    
    int fd_out = sys_open(argv[2], "w");
    if (fd_out < 0) {
        printf("Error: Cannot create destination %s\n", argv[2]);
        sys_close(fd_in);
        return 1;
    }
    
    char buffer[4096];
    int bytes;
    while ((bytes = sys_read(fd_in, buffer, sizeof(buffer))) > 0) {
        sys_write_fs(fd_out, buffer, bytes);
    }
    
    sys_close(fd_in);
    sys_close(fd_out);
    
    if (sys_delete(argv[1]) != 0) {
        printf("Warning: Failed to delete source %s after copy\n", argv[1]);
    }
    
    return 0;
}
