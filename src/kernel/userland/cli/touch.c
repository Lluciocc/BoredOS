// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
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
