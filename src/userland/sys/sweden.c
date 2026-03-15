// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

static void beep(int freq, int ms) {
    sys_system(14, freq, ms, 0, 0);
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Playing Sweden - C418 (Minecraft)...\n");
    
    // Main melody (simplified pattern)
    beep(392, 400); // G4
    beep(440, 400); // A4
    beep(493, 800); // B4
    
    beep(440, 400); // A4
    beep(392, 800); // G4
    beep(329, 800); // E4
    
    beep(392, 400); // G4
    beep(440, 400); // A4
    beep(493, 800); // B4
    
    beep(440, 400); // A4
    beep(392, 1200); // G4
    
    printf("Done.\n");
    return 0;
}
