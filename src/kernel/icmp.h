#ifndef ICMP_H
#define ICMP_H

#include <stdint.h>
#include "network.h"

void icmp_handle_packet(ipv4_address_t src, void *data, uint16_t len);
int cli_cmd_ping_syscall(ipv4_address_t *dest);

#endif
