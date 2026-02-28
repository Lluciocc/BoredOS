// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include <stdlib.h>
#include <syscall.h>

// Helper function to print a MAC address
static void print_mac(const net_mac_address_t* mac) {
    char buf[64];
    int p = 0;
    for (int i = 0; i < 6; i++) {
        int v = mac->bytes[i];
        int hi = (v >> 4) & 0xF;
        int lo = v & 0xF;
        buf[p++] = (hi < 10) ? ('0' + hi) : ('A' + (hi - 10));
        buf[p++] = (lo < 10) ? ('0' + lo) : ('A' + (lo - 10));
        if (i < 5) buf[p++] = ':';
    }
    buf[p] = 0;
    printf("%s", buf);
}

// Helper to parse integer from string
static int string_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result * sign;
}

// Helper to get string length
static int string_length(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// Command: netinit - Initialize network and acquire DHCP
static void cmd_netinit(void) {
    int r = sys_network_init();
    if (r == 0) {
        printf("Network initialized\n");
        int d = sys_network_dhcp_acquire();
        if (d == 0) {
            printf("DHCP acquired\n");
        } else {
            printf("DHCP failed\n");
        }
    } else {
        printf("Network init failed\n");
    }
}

// Command: netinfo - Display network information
static void cmd_netinfo(void) {
    if (!sys_network_is_initialized()) {
        printf("Error: Network not initialized. Run 'net init' first.\n");
        return;
    }
    
    net_mac_address_t mac;
    net_ipv4_address_t ip;
    
    if (sys_network_get_mac(&mac) == 0) {
        printf("MAC: ");
        print_mac(&mac);
        printf("\n");
    }
    
    if (sys_network_has_ip()) {
        if (sys_network_get_ip(&ip) == 0) {
            printf("IP: ");
            for (int i = 0; i < 4; i++) {
                printf("%d", ip.bytes[i]);
                if (i < 3) printf(".");
            }
            printf("\n");
        }
    } else {
        printf("IP: Not assigned\n");
    }
    
    printf("Frames: %d\n", sys_network_get_stat(0));
    printf("UDP packets: %d\n", sys_network_get_stat(1));
    printf("UDP callbacks: %d\n", sys_network_get_stat(2));
    printf("E1000 receive calls: %d\n", sys_network_get_stat(3));
    printf("E1000 receive empty: %d\n", sys_network_get_stat(4));
    printf("Process calls: %d\n", sys_network_get_stat(5));
}

// Command: ipset - Set IP address manually
static void cmd_ipset(const char *args) {
    if (!sys_network_is_initialized()) {
        printf("Error: Network not initialized. Run 'net init' first.\n");
        return;
    }
    
    if (!args || !*args) {
        printf("Usage: NET IPSET a.b.c.d\n");
        return;
    }
    
    net_ipv4_address_t ip = {{0, 0, 0, 0}};
    int part = 0;
    int val = 0;
    int i = 0;
    
    while (args[i]) {
        char ch = args[i++];
        if (ch >= '0' && ch <= '9') {
            val = val * 10 + (ch - '0');
            if (val > 255) {
                printf("Invalid IP\n");
                return;
            }
        } else if (ch == '.') {
            if (part > 3) {
                printf("Invalid IP\n");
                return;
            }
            ip.bytes[part++] = (uint8_t)val;
            val = 0;
        } else {
            printf("Invalid IP\n");
            return;
        }
    }
    
    if (part != 3) {
        printf("Invalid IP\n");
        return;
    }
    ip.bytes[3] = (uint8_t)val;
    
    if (sys_network_set_ip(&ip) == 0) {
        printf("IP set\n");
    } else {
        printf("IP set failed\n");
    }
}

// Command: udpsend - Send UDP packet
static void cmd_udpsend(const char *args) {
    if (!sys_network_is_initialized()) {
        printf("Error: Network not initialized. Run 'net init' first.\n");
        return;
    }
    
    if (!args || !*args) {
        printf("Usage: NET UDPSEND ip port data\n");
        return;
    }
    
    // Parse IP address
    char ipstr[32];
    int pos = 0;
    while (args[pos] && args[pos] != ' ') {
        ipstr[pos] = args[pos];
        pos++;
    }
    ipstr[pos] = 0;
    
    while (args[pos] == ' ') pos++;
    
    // Parse port
    char portstr[16];
    int p = 0;
    while (args[pos] && args[pos] != ' ') {
        portstr[p++] = args[pos++];
    }
    portstr[p] = 0;
    
    while (args[pos] == ' ') pos++;
    
    // Get data
    const char *datastr = args + pos;
    
    // Parse IP
    net_ipv4_address_t ip = {{0, 0, 0, 0}};
    int idx = 0;
    int val = 0;
    int j = 0;
    
    while (ipstr[j]) {
        char ch = ipstr[j++];
        if (ch >= '0' && ch <= '9') {
            val = val * 10 + (ch - '0');
            if (val > 255) {
                printf("Invalid IP\n");
                return;
            }
        } else if (ch == '.') {
            if (idx > 3) {
                printf("Invalid IP\n");
                return;
            }
            ip.bytes[idx++] = (uint8_t)val;
            val = 0;
        } else {
            printf("Invalid IP\n");
            return;
        }
    }
    
    if (idx != 3) {
        printf("Invalid IP\n");
        return;
    }
    ip.bytes[3] = (uint8_t)val;
    
    // Parse port
    int port = 0;
    int k = 0;
    while (portstr[k]) {
        char ch = portstr[k++];
        if (ch < '0' || ch > '9') {
            printf("Invalid port\n");
            return;
        }
        port = port * 10 + (ch - '0');
    }
    
    if (port <= 0 || port > 65535) {
        printf("Invalid port\n");
        return;
    }
    
    int len = string_length(datastr);
    if (len <= 0) {
        printf("No data\n");
        return;
    }
    
    int r = sys_udp_send(&ip, (uint16_t)port, 12345, datastr, (size_t)len);
    if (r == 0) {
        printf("Sent\n");
    } else {
        printf("Send failed\n");
    }
}

// Command: ping - Send ICMP ping request
static void cmd_ping(const char *args) {
    if (!sys_network_is_initialized()) {
        printf("Error: Network not initialized. Run 'net init' first.\n");
        return;
    }
    
    if (!args || !*args) {
        printf("Usage: NET PING a.b.c.d\n");
        return;
    }
    
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
                return;
            }
        } else if (ch == '.') {
            if (idx > 3) {
                printf("Invalid IP\n");
                return;
            }
            ip.bytes[idx++] = (uint8_t)val;
            val = 0;
        } else if (ch == ' ' || ch == '\t') {
            // Skip whitespace
            while (args[j] == ' ' || args[j] == '\t') j++;
            j--;
        } else {
            printf("Invalid IP\n");
            return;
        }
    }
    
    if (idx != 3) {
        printf("Invalid IP\n");
        return;
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
    } else if (result < 0) {
        printf("Error: Failed to send ping request.\n");
    } else {
        printf("Ping complete: %d/%d replies received\n", result, 4);
    }
}

// Command: help
static void cmd_help(void) {
    printf("Network Commands:\n");
    printf("  NET INIT     - Initialize network and acquire DHCP\n");
    printf("  NET INFO     - Display network information\n");
    printf("  NET IPSET a.b.c.d - Set IP address\n");
    printf("  NET UDPSEND ip port data - Send UDP packet\n");
    printf("  NET PING a.b.c.d - Send ICMP ping request\n");
    printf("  NET HELP     - Show this help\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cmd_help();
        return 1;
    }
    
    const char *cmd = argv[1];
    
    // Convert command to uppercase for easier comparison
    char cmd_upper[32];
    int i = 0;
    while (cmd[i] && i < 31) {
        char c = cmd[i];
        cmd_upper[i] = (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
        i++;
    }
    cmd_upper[i] = 0;
    
    // Parse arguments if provided
    const char *args = "";
    if (argc > 2) {
        args = argv[2];
    }
    
    if ((string_length(cmd_upper) == 4 && 
        cmd_upper[0] == 'I' && cmd_upper[1] == 'N' && cmd_upper[2] == 'I' && cmd_upper[3] == 'T') ||
        (string_length(cmd_upper) == 7 && 
        cmd_upper[0] == 'N' && cmd_upper[1] == 'E' && cmd_upper[2] == 'T' &&
        cmd_upper[3] == 'I' && cmd_upper[4] == 'N' && cmd_upper[5] == 'I' && cmd_upper[6] == 'T')) {
        cmd_netinit();
    } else if ((string_length(cmd_upper) == 4 &&
               cmd_upper[0] == 'I' && cmd_upper[1] == 'N' && cmd_upper[2] == 'F' && cmd_upper[3] == 'O') ||
               (string_length(cmd_upper) == 7 &&
               cmd_upper[0] == 'N' && cmd_upper[1] == 'E' && cmd_upper[2] == 'T' &&
               cmd_upper[3] == 'I' && cmd_upper[4] == 'N' && cmd_upper[5] == 'F' && cmd_upper[6] == 'O')) {
        cmd_netinfo();
    } else if (string_length(cmd_upper) == 5 &&
               cmd_upper[0] == 'I' && cmd_upper[1] == 'P' && cmd_upper[2] == 'S' &&
               cmd_upper[3] == 'E' && cmd_upper[4] == 'T') {
        cmd_ipset(args);
    } else if (string_length(cmd_upper) == 7 &&
               cmd_upper[0] == 'U' && cmd_upper[1] == 'D' && cmd_upper[2] == 'P' &&
               cmd_upper[3] == 'S' && cmd_upper[4] == 'E' && cmd_upper[5] == 'N' && cmd_upper[6] == 'D') {
        cmd_udpsend(args);
    } else if (string_length(cmd_upper) == 4 &&
               cmd_upper[0] == 'P' && cmd_upper[1] == 'I' && cmd_upper[2] == 'N' && cmd_upper[3] == 'G') {
        cmd_ping(args);
    } else if (string_length(cmd_upper) == 4 &&
               cmd_upper[0] == 'H' && cmd_upper[1] == 'E' && cmd_upper[2] == 'L' && cmd_upper[3] == 'P') {
        cmd_help();
    } else {
        printf("Unknown command: %s\n", cmd);
        cmd_help();
        return 1;
    }
    
    return 0;
}
