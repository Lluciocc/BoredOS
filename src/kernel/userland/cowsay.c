#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    char *msg = (char*)"Bored!";
    if (argc > 1) {
        // Simple concatenation of args for now
        // For simplicity in this demo, just use the first arg
        msg = argv[1];
    }
    
    size_t len = strlen(msg);
    
    printf(" ");
    for(size_t i=0; i<len+2; i++) printf("_");
    printf("\n< %s >\n ", msg);
    for(size_t i=0; i<len+2; i++) printf("-");
    printf("\n");
    printf("        \\   ^__^\n");
    printf("         \\  (oo)\\_______\n");
    printf("            (__)\\       )\\/\\\n");
    printf("                ||----w |\n");
    printf("                ||     ||\n\n");
    
    return 0;
}
