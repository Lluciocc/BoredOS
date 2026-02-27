// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
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
