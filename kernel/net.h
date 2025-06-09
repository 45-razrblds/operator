#ifndef NET_H
#define NET_H

#include <stddef.h>
#include <stdint.h>

// Ethernet frame header
struct eth_header {
    uint8_t  dest_mac[6];
    uint8_t  src_mac[6];
    uint16_t ethertype;
} __attribute__((packed));

// IPv4 header
struct ip_header {
    uint8_t  version_ihl;
    uint8_t  tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_fragoffset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed));

// ICMP header
struct icmp_header {
    uint8_t  type;
    uint8_t  code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
} __attribute__((packed));

// ARP header
struct arp_header {
    uint16_t hw_type;         // Hardware type (1 for Ethernet)
    uint16_t proto_type;      // Protocol type (0x0800 for IPv4)
    uint8_t  hw_len;         // Hardware address length (6 for MAC)
    uint8_t  proto_len;      // Protocol address length (4 for IPv4)
    uint16_t opcode;         // Operation (1=request, 2=reply)
    uint8_t  sender_mac[6];  // Sender MAC address
    uint32_t sender_ip;      // Sender IP address
    uint8_t  target_mac[6];  // Target MAC address
    uint32_t target_ip;      // Target IP address
} __attribute__((packed));

// Initialize network stack
void net_init(void);

// Send raw network data
int net_send(const void* data, size_t len);

// Receive raw network data
int net_receive(void* buf, size_t maxlen);

// Check if network is initialized
int net_is_initialized(void);

// Calculate IP/ICMP checksum
uint16_t net_checksum(const void* data, size_t len);

// Send ICMP echo request (ping)
int net_ping(uint32_t dest_ip);

// ARP functions
int net_arp_resolve(uint32_t ip_addr, uint8_t mac_out[6]);
void net_arp_update(uint32_t ip_addr, const uint8_t mac[6]);

#endif // NET_H
