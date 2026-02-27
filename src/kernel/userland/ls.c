// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    char path[256];
    if (argc > 1) {
        strcpy(path, argv[1]);
    } else {
        if (!getcwd(path, sizeof(path))) {
            strcpy(path, ".");
        }
    }
    
    FAT32_FileInfo entries[128];
    int count = sys_list(path, entries, 128);
    
    if (count < 0) {
        printf("Error: Cannot list directory %s\n", path);
        return 1;
    }
    
    for (int i = 0; i < count; i++) {
        if (entries[i].is_directory) {
            printf("[DIR]  %s\n", entries[i].name);
        } else {
            printf("[FILE] %s (%d bytes)\n", entries[i].name, entries[i].size);
        }
    }
    
    printf("\nTotal: %d items\n", count);
    return 0;
}
