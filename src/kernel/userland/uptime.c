#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    uint64_t ticks = sys_system(16, 0, 0, 0, 0); // SYSTEM_CMD_UPTIME
    uint64_t seconds = ticks / 100; // 100Hz timer assumed
    uint64_t minutes = seconds / 60;
    uint64_t hours = minutes / 60;
    uint64_t days = hours / 24;
    
    printf("Uptime: %d days, %d hours, %d minutes, %d seconds\n", 
           (int)days, (int)(hours % 24), (int)(minutes % 60), (int)(seconds % 60));
    
    return 0;
}
