// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    uint32_t error_color = (uint32_t)sys_get_shell_config("error_color");
    uint32_t success_color = (uint32_t)sys_get_shell_config("success_color");
    uint32_t default_color = (uint32_t)sys_get_shell_config("default_text_color");

    if (argc < 2) {
        printf("Usage: rm <path>\n");
        return 1;
    }
    
    // Simple rm (no recursive support yet for simplicity, but can be added)
    if (sys_delete(argv[1]) == 0) {
        sys_set_text_color(success_color);
        printf("Deleted: %s\n", argv[1]);
    } else {
        sys_set_text_color(error_color);
        printf("Error: Cannot delete %s\n", argv[1]);
        sys_set_text_color(default_color);
        return 1;
    }
    sys_set_text_color(default_color);
    return 0;
}
