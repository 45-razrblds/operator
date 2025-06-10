#include <stddef.h>
#include "terminal.h"

// Lock all network commands
void net_init(void) {}
int net_send(const void* data, size_t len) { return -1; }
int net_receive(void* buf, size_t maxlen) { return -1; }
int net_is_initialized(void) { return 0; }
int net_ping(uint32_t dest_ip) { return -1; }
void net_arp_update(uint32_t ip_addr, const uint8_t mac[6]) {}
int net_arp_resolve(uint32_t ip_addr, uint8_t mac_out[6]) { return -1; }
uint16_t net_checksum(const void* data, size_t len) { return 0; }
