// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Shutting down...\n");
    sys_system(13, 0, 0, 0, 0); // SYSTEM_CMD_SHUTDOWN
    return 0;
}
