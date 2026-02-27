#include <stdlib.h>
#include <syscall.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ping a.b.c.d\n");
        return 1;
    }
    
    if (!sys_network_is_initialized()) {
        printf("Error: Network not initialized. Run 'net init' first.\n");
        return 1;
    }
    
    const char *args = argv[1];
    
    net_ipv4_address_t ip = {{0, 0, 0, 0}};
    int idx = 0;
    int val = 0;
    int j = 0;
    
    while (args[j]) {
        char ch = args[j++];
        if (ch >= '0' && ch <= '9') {
            val = val * 10 + (ch - '0');
            if (val > 255) {
                printf("Invalid IP\n");
                return 1;
            }
        } else if (ch == '.') {
            if (idx > 3) {
                printf("Invalid IP\n");
                return 1;
            }
            ip.bytes[idx++] = (uint8_t)val;
            val = 0;
        } else if (ch == ' ' || ch == '\t') {
            // Skip whitespace
            while (args[j] == ' ' || args[j] == '\t') j++;
            j--;
        } else {
            printf("Invalid IP\n");
            return 1;
        }
    }
    
    if (idx != 3) {
        printf("Invalid IP\n");
        return 1;
    }
    ip.bytes[3] = (uint8_t)val;
    
    printf("Pinging ");
    for (int i = 0; i < 4; i++) {
        printf("%d", ip.bytes[i]);
        if (i < 3) printf(".");
    }
    printf("...\n");
    
    int result = sys_icmp_ping(&ip);
    
    if (result == -2) {
        printf("Error: Network not initialized. Run 'net init' first.\n");
        return 1;
    } else if (result < 0) {
        printf("Error: Failed to send ping request.\n");
        return 1;
    }
    
    printf("Ping complete: %d/%d replies received\n", result, 4);
    
    return 0;
}
