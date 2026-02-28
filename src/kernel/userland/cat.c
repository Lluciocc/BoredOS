// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    uint32_t error_color = (uint32_t)sys_get_shell_config("error_color");
    uint32_t default_color = (uint32_t)sys_get_shell_config("default_text_color");

    if (argc < 2) {
        printf("Usage: cat <filename>\n");
        return 1;
    }
    
    int fd = sys_open(argv[1], "r");
    if (fd < 0) {
        sys_set_text_color(error_color);
        printf("Error: Cannot open %s\n", argv[1]);
        sys_set_text_color(default_color);
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
