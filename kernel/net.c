#include "net.h"
#include "rtl8139.h"
#include "terminal.h"
#include "timer.h"
#include "minilibc.h"

static int initialized = 0;

// Our MAC address (hard-coded for now)
static const uint8_t our_mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};

// Our IP address (hard-coded for now, using 10.0.2.15 which is QEMU's default)
static const uint32_t our_ip = 0x0F02000A;  // 10.0.2.15 in network byte order

// ARP cache structure
#define ARP_CACHE_SIZE 16
struct arp_entry {
    uint32_t ip;
    uint8_t mac[6];
    uint32_t time;
    uint8_t valid;
} static arp_cache[ARP_CACHE_SIZE];

// Forward declarations
static char* dec_to_str(int num);
static void print_ip(uint32_t ip);
static void print_hex(uint8_t byte);
static int send_arp_request(uint32_t target_ip);
static void handle_arp_packet(const uint8_t* packet, size_t len);

uint16_t net_checksum(const void* data, size_t len) {
    const uint16_t* buf = data;
    uint32_t sum = 0;
    
    // Sum up 16-bit words
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    
    // Add left-over byte if any
    if (len > 0) {
        sum += *(uint8_t*)buf;
    }
    
    // Fold 32-bit sum into 16 bits
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    
    return ~sum;
}

static char* dec_to_str(int num) {
    static char buf[16];
    int i = 0;
    if (num == 0) buf[i++] = '0';
    else {
        int rev = 0;
        while (num > 0) {
            rev = rev * 10 + (num % 10);
            num /= 10;
        }
        while (rev > 0) {
            buf[i++] = '0' + (rev % 10);
            rev /= 10;
        }
    }
    buf[i] = 0;
    return buf;
}

static void print_ip(uint32_t ip) {
    terminal_writestring(dec_to_str((ip >> 24) & 0xFF));
    terminal_putchar('.');
    terminal_writestring(dec_to_str((ip >> 16) & 0xFF));
    terminal_putchar('.');
    terminal_writestring(dec_to_str((ip >> 8) & 0xFF));
    terminal_putchar('.');
    terminal_writestring(dec_to_str(ip & 0xFF));
}

static void print_hex(uint8_t byte) {
    char hexbuf[3];
    int nibble = byte >> 4;
    hexbuf[0] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    nibble = byte & 0xF;
    hexbuf[1] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    hexbuf[2] = 0;
    terminal_writestring(hexbuf);
}

int net_ping(uint32_t dest_ip) {
    uint8_t packet[64];
    struct eth_header* eth = (struct eth_header*)packet;
    struct ip_header* ip = (struct ip_header*)(packet + sizeof(struct eth_header));
    struct icmp_header* icmp = (struct icmp_header*)(packet + sizeof(struct eth_header) + sizeof(struct ip_header));
    
    terminal_writestring("Sending ping to ");
    print_ip(dest_ip);
    terminal_writestring("...\n");
    
    // Resolve destination MAC address
    uint8_t dest_mac[6];
    if (net_arp_resolve(dest_ip, dest_mac) < 0) {
        terminal_writestring("Failed to resolve MAC address\n");
        return -1;
    }
    
    // Setup Ethernet header
    memcpy(eth->dest_mac, dest_mac, 6);
    memcpy(eth->src_mac, our_mac, 6);
    eth->ethertype = 0x0008;  // IPv4 in network byte order
    
    // Setup IP header
    ip->version_ihl = 0x45;   // IPv4, 20-byte header
    ip->tos = 0;
    ip->total_length = sizeof(struct ip_header) + sizeof(struct icmp_header);
    ip->id = 0x1234;
    ip->flags_fragoffset = 0;
    ip->ttl = 64;
    ip->protocol = 1;  // ICMP
    ip->checksum = 0;
    ip->src_ip = our_ip;
    ip->dest_ip = dest_ip;
    ip->checksum = net_checksum(ip, sizeof(struct ip_header));
    
    // Setup ICMP header
    icmp->type = 8;  // Echo request
    icmp->code = 0;
    icmp->checksum = 0;
    icmp->id = 0x4321;
    icmp->sequence = 1;
    icmp->checksum = net_checksum(icmp, sizeof(struct icmp_header));
    
    terminal_writestring("Packet contents:\n");
    terminal_writestring("  MAC: ");
    for (int i = 0; i < 6; i++) {
        if (i > 0) terminal_putchar(':');
        char hexbuf[3];
        int nibble = eth->dest_mac[i] >> 4;
        hexbuf[0] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
        nibble = eth->dest_mac[i] & 0xF;
        hexbuf[1] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
        hexbuf[2] = 0;
        terminal_writestring(hexbuf);
    }
    terminal_writestring("\n  IP: ");
    print_ip(ip->dest_ip);
    terminal_writestring("\n  ICMP: type=");
    terminal_writestring(dec_to_str(icmp->type));
    terminal_writestring(" code=");
    terminal_writestring(dec_to_str(icmp->code));
    terminal_writestring(" id=");
    terminal_writestring(dec_to_str(icmp->id));
    terminal_writestring("\n");
    
    // Send packet
    int sent = net_send(packet, sizeof(struct eth_header) + ip->total_length);
    if (sent <= 0) {
        terminal_writestring("Failed to send ping request\n");
        return -1;
    }
    
    terminal_writestring("Ping sent, waiting for reply...\n");
    
    // Wait for reply
    int tries = 5;
    while (tries--) {
        uint8_t reply[1518];  // Max Ethernet frame size
        int received = net_receive(reply, sizeof(reply));
        
        if (received > 0) {
            struct eth_header* eth_reply = (struct eth_header*)reply;
            struct ip_header* ip_reply = (struct ip_header*)(reply + sizeof(struct eth_header));
            struct icmp_header* icmp_reply = (struct icmp_header*)(reply + sizeof(struct eth_header) + sizeof(struct ip_header));
            
            // Check if it's an ICMP echo reply for us
            if (eth_reply->ethertype == 0x0008 &&
                ip_reply->protocol == 1 &&
                ip_reply->dest_ip == our_ip &&
                icmp_reply->type == 0 &&  // Echo reply
                icmp_reply->id == 0x4321) {
                    terminal_writestring("Received ping reply!\n");
                    return 0;
            }
        }
        
        delay_ms(1000);  // Wait 1 second between tries
    }
    
    terminal_writestring("No ping reply received\n");
    return -1;
}

void net_init(void) {
    rtl8139_init();
    initialized = 1;
}

int net_send(const void* data, size_t len) {
    if (!initialized) return -1;
    return rtl8139_send(data, len);
}

int net_receive(void* buf, size_t maxlen) {
    if (!initialized) return -1;
    return rtl8139_receive(buf, maxlen);
}

int net_is_initialized(void) {
    return initialized;
}

void net_arp_update(uint32_t ip_addr, const uint8_t mac[6]) {
    // Find existing entry or empty slot
    int empty_slot = -1;
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip_addr) {
            // Update existing entry
            memcpy(arp_cache[i].mac, mac, 6);
            arp_cache[i].time = get_system_ticks();
            return;
        }
        if (!arp_cache[i].valid && empty_slot == -1) {
            empty_slot = i;
        }
    }
    
    // If not found, use empty slot
    if (empty_slot != -1) {
        arp_cache[empty_slot].ip = ip_addr;
        memcpy(arp_cache[empty_slot].mac, mac, 6);
        arp_cache[empty_slot].time = get_system_ticks();
        arp_cache[empty_slot].valid = 1;
    }
}

int net_arp_resolve(uint32_t ip_addr, uint8_t mac_out[6]) {
    // Check cache first
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (arp_cache[i].valid && arp_cache[i].ip == ip_addr) {
            // Check if entry is not too old (30 seconds)
            if (get_system_ticks() - arp_cache[i].time < 300) {
                memcpy(mac_out, arp_cache[i].mac, 6);
                return 0;
            }
            // Invalidate old entry
            arp_cache[i].valid = 0;
            break;
        }
    }
    
    // Send ARP request and wait for reply
    if (send_arp_request(ip_addr) < 0) {
        return -1;
    }
    
    // Wait for reply with timeout
    int tries = 5;
    while (tries--) {
        uint8_t reply[1518];
        int received = net_receive(reply, sizeof(reply));
        
        if (received > 0) {
            handle_arp_packet(reply, received);
            
            // Check if we got the MAC now
            for (int i = 0; i < ARP_CACHE_SIZE; i++) {
                if (arp_cache[i].valid && arp_cache[i].ip == ip_addr) {
                    memcpy(mac_out, arp_cache[i].mac, 6);
                    return 0;
                }
            }
        }
        
        delay_ms(200);  // Wait 200ms between tries
    }
    
    return -1;
}

static int send_arp_request(uint32_t target_ip) {
    uint8_t packet[42];  // Ethernet(14) + ARP(28)
    struct eth_header* eth = (struct eth_header*)packet;
    struct arp_header* arp = (struct arp_header*)(packet + sizeof(struct eth_header));
    
    // Setup Ethernet header
    memcpy(eth->dest_mac, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);  // Broadcast
    memcpy(eth->src_mac, our_mac, 6);
    eth->ethertype = 0x0608;  // ARP in network byte order
    
    // Setup ARP header
    arp->hw_type = 0x0100;     // Ethernet in network byte order
    arp->proto_type = 0x0008;  // IPv4 in network byte order
    arp->hw_len = 6;          // MAC address length
    arp->proto_len = 4;       // IPv4 address length
    arp->opcode = 0x0100;     // Request in network byte order
    memcpy(arp->sender_mac, our_mac, 6);
    arp->sender_ip = our_ip;
    memset(arp->target_mac, 0, 6);
    arp->target_ip = target_ip;
    
    terminal_writestring("Sending ARP request for ");
    print_ip(target_ip);
    terminal_writestring("\n");
    
    return net_send(packet, sizeof(packet));
}

static void handle_arp_packet(const uint8_t* packet, size_t len) {
    if (len < sizeof(struct eth_header) + sizeof(struct arp_header)) {
        return;  // Packet too small
    }
    
    struct eth_header* eth = (struct eth_header*)packet;
    struct arp_header* arp = (struct arp_header*)(packet + sizeof(struct eth_header));
    
    // Check if it's an ARP packet
    if (eth->ethertype != 0x0608) {  // ARP in network byte order
        return;
    }
    
    // Check protocol type and lengths
    if (arp->hw_type != 0x0100 ||   // Ethernet
        arp->proto_type != 0x0008 || // IPv4
        arp->hw_len != 6 ||         // MAC
        arp->proto_len != 4) {      // IPv4
        return;
    }
    
    // Update ARP cache with sender's info
    net_arp_update(arp->sender_ip, arp->sender_mac);
    
    // If it's a request for our IP, send a reply
    if (arp->opcode == 0x0100 && arp->target_ip == our_ip) {
        // Swap ethernet addresses
        memcpy(eth->dest_mac, eth->src_mac, 6);
        memcpy(eth->src_mac, our_mac, 6);
        
        // Convert to reply
        arp->opcode = 0x0200;  // Reply in network byte order
        memcpy(arp->target_mac, arp->sender_mac, 6);
        arp->target_ip = arp->sender_ip;
        memcpy(arp->sender_mac, our_mac, 6);
        arp->sender_ip = our_ip;
        
        terminal_writestring("Sending ARP reply to ");
        print_ip(arp->target_ip);
        terminal_writestring("\n");
        
        net_send(packet, sizeof(struct eth_header) + sizeof(struct arp_header));
    }
}
