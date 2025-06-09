#include "command.h"
#include "terminal.h"
#include "debug.h"
#include "opfs.h"
#include "shutdown.h"
#include "reboot.h"
#include "minilibc.h"
#include "serial.h"
#include "net.h"
#include "timer.h"
#include "timer.h"

// ICMP echo (ping) packet structure
typedef struct {
    uint8_t type;      // ICMP type
    uint8_t code;      // ICMP subtype
    uint16_t checksum; // Internet checksum
    uint16_t id;       // Identifier
    uint16_t seq;      // Sequence number
    uint8_t data[32];  // Data
} __attribute__((packed)) icmp_packet_t;

// IP header structure
typedef struct {
    uint8_t ver_ihl;   // Version and IHL
    uint8_t tos;       // Type of Service
    uint16_t len;      // Total length
    uint16_t id;       // ID number
    uint16_t frag;     // Fragment offset
    uint8_t ttl;       // Time To Live
    uint8_t proto;     // Protocol
    uint16_t csum;     // Header checksum
    uint32_t sip;      // Source IP
    uint32_t dip;      // Destination IP
} __attribute__((packed)) ip_header_t;

// Calculate Internet checksum
static uint16_t checksum(void* addr, int count) {
    uint16_t* ptr = addr;
    uint32_t sum = 0;
    
    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }
    
    if (count > 0)
        sum += *((uint8_t*)ptr);
    
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    
    return ~sum;
}

char input_buffer[256];
int input_pos = 0;
extern int debug_mode;

void process_command() {
    // Trim leading/trailing whitespace
    char* start = input_buffer;
    char* end = input_buffer + strlen(input_buffer) - 1;
    
    while (*start == ' ' || *start == '\t') start++;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = 0;
    
    if (start != input_buffer) {
        memmove(input_buffer, start, strlen(start) + 1);
    }

    if (strncmp(input_buffer, "ping", 4) == 0) {
        // Create an ICMP echo request packet
        uint8_t packet[sizeof(ip_header_t) + sizeof(icmp_packet_t)];
        ip_header_t* ip = (ip_header_t*)packet;
        icmp_packet_t* icmp = (icmp_packet_t*)(packet + sizeof(ip_header_t));
        
        // Fill IP header
        ip->ver_ihl = 0x45;  // IPv4, 5 words
        ip->tos = 0;
        ip->len = sizeof(packet);
        ip->id = 0;
        ip->frag = 0;
        ip->ttl = 64;
        ip->proto = 1;  // ICMP
        ip->csum = 0;
        ip->sip = 0x0A000002;  // 10.0.0.2
        ip->dip = 0x0A000001;  // 10.0.0.1 (gateway)
        ip->csum = checksum(ip, sizeof(ip_header_t));
        
        // Fill ICMP packet
        icmp->type = 8;  // Echo request
        icmp->code = 0;
        icmp->checksum = 0;
        icmp->id = 1;
        icmp->seq = 1;
        memset(icmp->data, 0x41, sizeof(icmp->data));  // Fill with 'A's
        icmp->checksum = checksum(icmp, sizeof(icmp_packet_t));
        
        terminal_writestring("Sending ping to 10.0.0.1...\n");
        if (net_send(packet, sizeof(packet)) > 0) {
            terminal_writestring("Ping sent, waiting for reply...\n");
            
            // Wait for reply with timeout
            uint8_t reply[sizeof(packet)];
            int tries = 10;  // 1 second timeout
            while (tries-- > 0) {
                int len = net_receive(reply, sizeof(reply));
                if (len > 0) {
                    ip_header_t* rip = (ip_header_t*)reply;
                    icmp_packet_t* ricmp = (icmp_packet_t*)(reply + sizeof(ip_header_t));
                    if (ricmp->type == 0) {  // Echo reply
                        terminal_set_color(0x0A);
                        terminal_writestring("Reply received!\n");
                        terminal_set_color(0x07);
                        return;
                    }
                }
                delay_ms(100);  // Wait 100ms between tries
            }
            terminal_set_color(0x0C);
            terminal_writestring("No reply received (timeout)\n");
            terminal_set_color(0x07);
        } else {
            terminal_set_color(0x0C);
            terminal_writestring("Failed to send ping\n");
            terminal_set_color(0x07);
        }
        return;
    } else if (strncmp(input_buffer, "ls", 2) == 0) {
        char* arg = input_buffer+2; while (*arg == ' ') arg++;
        opfs_ls(*arg ? arg : NULL);
    } else if (strncmp(input_buffer, "cat ", 4) == 0) {
        opfs_cat(input_buffer+4);
    } else if (strncmp(input_buffer, "edit ", 5) == 0) {
        opfs_edit(input_buffer+5);
    } else if (strncmp(input_buffer, "touch ", 6) == 0) {
        opfs_touch(input_buffer+6);
    } else if (strncmp(input_buffer, "rm ", 3) == 0) {
        opfs_rm(input_buffer+3);
    } else if (strncmp(input_buffer, "mkdir ", 6) == 0) {
        opfs_mkdir(input_buffer+6);
    } else if (strncmp(input_buffer, "rmdir ", 6) == 0) {
        opfs_rm(input_buffer+6);
    } else if (strncmp(input_buffer, "cd ", 3) == 0) {
        opfs_cd(input_buffer+3);
    } else if (strncmp(input_buffer, "pwd", 3) == 0) {
        opfs_pwd();
    } else if (strncmp(input_buffer, "debug", 5) == 0) {
        toggle_debug_mode();
    } else if (strncmp(input_buffer, "shutdown", 8) == 0) {
        terminal_set_color(0x0C);
        terminal_writestring("\nShutting down...\n");
        terminal_set_color(0x07);
        // delay(100); // Optionally add delay
        shutdown();
    } else if (strncmp(input_buffer, "reboot", 6) == 0) {
        terminal_set_color(0x0C);
        terminal_writestring("\nRebooting...\n");
        terminal_set_color(0x07);
        // delay(100); // Optionally add delay
        reboot();
    } else if (strncmp(input_buffer, "help", 4) == 0) {
        terminal_set_color(0x0F);
        terminal_writestring("Available commands:\n");
        terminal_set_color(0x07);
        terminal_writestring("  ls [dir]       - List files and directories\n");
        terminal_writestring("  cat <file>     - Display file contents\n");
        terminal_writestring("  edit <file>    - Edit file contents (ESC to save and exit)\n");
        terminal_writestring("  touch <file>   - Create a new empty file\n");
        terminal_writestring("  rm <file|dir>  - Remove a file or directory\n");
        terminal_writestring("  mkdir <dir>    - Create a new directory\n");
        terminal_writestring("  cd <dir>       - Change current directory\n");
        terminal_writestring("  pwd            - Print working directory\n");
        terminal_writestring("  clear          - Clear the screen\n");
        terminal_writestring("  debug          - Toggle debug mode\n");
        terminal_writestring("  shutdown       - Power off the system\n");
        terminal_writestring("  reboot         - Restart the system\n");
        terminal_writestring("  help           - Show this help message\n");
        terminal_writestring("  ping           - Send a network ping request\n");
        terminal_writestring("  netstat        - Display network status\n");
    } else if (strncmp(input_buffer, "ping", 4) == 0) {
        if (!net_is_initialized()) {
            terminal_set_color(0x0C);
            terminal_writestring("Network not initialized\n");
            terminal_set_color(0x07);
            return;
        }
        
        char* arg = input_buffer + 4;
        while (*arg == ' ') arg++;
        
        if (!*arg) {
            terminal_writestring("Usage: ping <ip>\n");
            terminal_writestring("Example: ping 10.0.2.2\n");
            return;
        }
        
        // Parse IP address (e.g., "10.0.2.2")
        uint32_t ip = 0;
        int octet = 0;
        uint8_t* ip_bytes = (uint8_t*)&ip;
        char* start = arg;
        
        for (int i = 0; i < 4; i++) {
            while (*arg && *arg != '.') {
                if (*arg < '0' || *arg > '9') {
                    terminal_writestring("Invalid IP address\n");
                    return;
                }
                octet = octet * 10 + (*arg - '0');
                arg++;
            }
            
            if (octet > 255) {
                terminal_writestring("Invalid IP address\n");
                return;
            }
            
            ip_bytes[i] = octet;
            octet = 0;
            
            if (i < 3) {
                if (*arg != '.') {
                    terminal_writestring("Invalid IP address format\n");
                    return;
                }
                arg++;
            }
        }
        
        terminal_writestring("Pinging ");
        terminal_writestring(start);
        terminal_writestring("...\n");
        
        if (net_ping(ip) == 0) {
            terminal_set_color(0x0A);
            terminal_writestring("Ping successful!\n");
        } else {
            terminal_set_color(0x0C);
            terminal_writestring("Ping failed!\n");
        }
        terminal_set_color(0x07);
    } else if (strncmp(input_buffer, "netstat", 7) == 0) {
        // Display network interface status
        terminal_writestring("Network Interface Status:\n");
        terminal_writestring("----------------------\n");
        terminal_writestring("Device: RTL8139\n");
        terminal_writestring("Status: ");
        if (net_is_initialized()) {
            terminal_set_color(0x0A);
            terminal_writestring("Connected\n");
            terminal_set_color(0x07);
            terminal_writestring("IP: 10.0.2.15\n");
            terminal_writestring("MAC: 52:54:00:12:34:56\n");
        } else {
            terminal_set_color(0x0C);
            terminal_writestring("Disconnected\n");
        }
        terminal_set_color(0x07);
    } else if (strncmp(input_buffer, "clear", 5) == 0) {
        terminal_initialize();
        // draw_banner();
    } else {
        terminal_writestring("Unknown command. Type 'help'.\n");
    }
    input_pos = 0;
    memset(input_buffer, 0, 256);
    if (debug_mode) draw_debug_info();
}
