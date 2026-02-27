#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    char path[256];
    if (getcwd(path, sizeof(path))) {
        printf("%s\n", path);
    } else {
        printf("Error: Could not get current directory\n");
        return 1;
    }
    return 0;
}
