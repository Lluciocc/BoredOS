#include <stdlib.h>
#include <syscall.h>

int main(int argc, char** argv) {
    printf("Hello from Userland ELF!\n");
    
    printf("argc: %d\n", argc);
    
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    
    return 0;
}
