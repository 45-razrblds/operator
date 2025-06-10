#include "rtl8139.h"
#include "terminal.h"
#include "io.h"
#include "memory.h"
#include "minilibc.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

// PCI configuration registers
#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// RTL8139 PCI configuration registers
#define PCI_COMMAND     0x04
#define PCI_BAR0       0x10
#define PCI_INTERRUPT  0x3C

// RTL8139 device registers
#define CONFIG_1     0x52    /* Config register 1 */
#define CMD         0x37    /* Command register */
#define CAPR        0x38    /* Current Address of Packet Read */
#define RX_BUF      0x30    /* Receive Buffer Start Address */
#define RX_EARLY_CNT 0x34    /* Early Rx Byte Count */
#define RX_EARLY_STATUS 0x36 /* Early Rx Status */
#define IMR         0x3C    /* Interrupt Mask Register */
#define ISR         0x3E    /* Interrupt Status Register */
#define TCR         0x40    /* Transmit Configuration Register */
#define RCR         0x44    /* Receive Configuration Register */
#define TX_STATUS0  0x10    /* Transmit Status Register 0 */
#define TX_ADDR0    0x20    /* Transmit Address 0 */
#define TX_STATUS1  0x14    /* Transmit Status Register 1 */
#define TX_ADDR1    0x24    /* Transmit Address 1 */
#define TX_STATUS2  0x18    /* Transmit Status Register 2 */
#define TX_ADDR2    0x28    /* Transmit Address 2 */
#define TX_STATUS3  0x1C    /* Transmit Status Register 3 */
#define TX_ADDR3    0x2C    /* Transmit Address 3 */

// Register bits
#define CMD_RESET   0x10
#define CMD_RX_ENABLE  0x08
#define CMD_TX_ENABLE  0x04

// Transmit Status bits
#define TX_HOST_OWNS    0x2000
#define TX_UNDERRUN     0x4000
#define TX_STAT_OK      0x8000
#define TX_OUT_OF_WINDOW    0x20000000
#define TX_ABORTED      0x40000000
#define TX_CARRIER_LOST     0x80000000

// Receive Status bits
#define RX_ROK          0x01
#define RX_FAE          0x02
#define RX_CRC          0x04
#define RX_LONG         0x08
#define RX_RUNT         0x10
#define RX_INVALID      0x20
#define RX_OK           0x0001
#define RX_BAD          0x0002
#define RX_FIFOOVER     0x0004

// Interrupt bits
#define INT_ROK         0x0001
#define INT_TOK         0x0004
#define INT_RXERR       0x0002
#define INT_TXERR       0x0008
#define INT_RX_BUFF_OF  0x0010
#define INT_LINK_CHG    0x0020
#define INT_RX_FIFO_OF  0x0040
#define INT_LEN_CHG     0x2000
#define INT_SYSTEM_ERR  0x8000

#define TCR_IFG96        0x03000000  /* Inter-frame gap time */
#define TCR_MXDMA_2048  0x00600000  /* Max DMA burst size 2048 bytes */
#define TCR_CRC         0x00010000  /* Append CRC */
#define TCR_RETRY_TX     0x00040000  /* Enable TX retry */
#define TCR_CLEAR_ABORT  0x00000001  /* Clear abort */
#define TCR_NORMAL      (TCR_IFG96 | TCR_MXDMA_2048 | TCR_CRC | TCR_RETRY_TX)

#define RCR_ACCEPT_ALL   0x0000000F  /* Accept all packets */
#define RCR_AB          0x00000008  /* Accept Broadcast packets */
#define RCR_AM          0x00000004  /* Accept Multicast packets */
#define RCR_APM         0x00000002  /* Accept Physical Match packets */
#define RCR_AAP         0x00000001  /* Accept All Packets */
#define RCR_MXDMA_1024  0x00000600  /* Max DMA burst size (1024 bytes) */
#define RCR_RXFTH_1     0x00008000  /* Receive FIFO Threshold (1 byte) */
#define RCR_NORMAL      (RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_MXDMA_1024 | RCR_RXFTH_1)

static uint16_t iobase;
static uint8_t* rx_buffer;
static int rx_offset;

static uint32_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) {
    uint32_t addr = (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);
    outl(PCI_CONFIG_ADDR, addr);
    return inl(PCI_CONFIG_DATA);
}

static void pci_write_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t addr = (uint32_t)((bus << 16) | (dev << 11) | (func << 8) | (offset & 0xfc) | 0x80000000);
    outl(PCI_CONFIG_ADDR, addr);
    outl(PCI_CONFIG_DATA, value);
}

static int find_rtl8139(void) {
    for(uint16_t bus = 0; bus < 256; bus++) {
        for(uint16_t dev = 0; dev < 32; dev++) {
            for(uint16_t func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_read_config(bus, dev, func, 0);
                uint16_t vendor = vendor_device & 0xFFFF;
                uint16_t device = (vendor_device >> 16) & 0xFFFF;
                
                if(vendor == RTL8139_VENDOR_ID && device == RTL8139_DEVICE_ID) {
                    iobase = pci_read_config(bus, dev, func, PCI_BAR0) & 0xFFFFFFFC;
                    
                    // Enable bus mastering and I/O space
                    uint16_t command = pci_read_config(bus, dev, func, PCI_COMMAND);
                    command |= 0x5;  // Enable bus mastering and I/O space
                    pci_write_config(bus, dev, func, PCI_COMMAND, command);
                    
                    return 1;
                }
            }
        }
    }
    return 0;
}

void rtl8139_init(void) {
    terminal_writestring("[rtl8139] Initializing network card\n");
    
    if (!find_rtl8139()) {
        terminal_writestring("[rtl8139] Device not found!\n");
        return;
    }
    
    terminal_writestring("[rtl8139] Device found at I/O base 0x");
    char hexbuf[9];
    for (int i = 0; i < 8; i++) {
        int nibble = (iobase >> (28 - i*4)) & 0xF;
        hexbuf[i] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    }
    hexbuf[8] = '\0';
    terminal_writestring(hexbuf);
    terminal_writestring("\n");
    
    // Perform software reset
    outb(iobase + CMD, CMD_RESET);
    
    // Wait for reset to complete
    int timeout = 1000;
    while ((inb(iobase + CMD) & CMD_RESET) != 0) {
        if (--timeout == 0) {
            terminal_writestring("[rtl8139] Reset timeout!\n");
            return;
        }
    }
    
    // Allocate receive buffer (8K + 16 bytes for overflow)
    rx_buffer = malloc(8192 + 16);
    if (!rx_buffer) {
        terminal_writestring("[rtl8139] Failed to allocate RX buffer!\n");
        return;
    }
    
    memset(rx_buffer, 0, 8192 + 16);
    terminal_writestring("[rtl8139] RX buffer allocated at 0x");
    for (int i = 0; i < 8; i++) {
        int nibble = ((uint32_t)rx_buffer >> (28 - i*4)) & 0xF;
        hexbuf[i] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
    }
    terminal_writestring(hexbuf);
    terminal_writestring("\n");
    
    // Set Rx Buffer
    outl(iobase + RX_BUF, (uint32_t)rx_buffer);
    
    // Write configuration register (0x52)
    // 0x20 = Auto-negotiate, 0x08 = Enable Auto-Negotiation
    outb(iobase + CONFIG_1, 0x28);
    
    // Configure transmit settings:
    // - Use higher DMA burst size
    // - Enable auto-retry on error
    // - Set IFG to recommended value
    uint32_t tcr_value = TCR_IFG96 | TCR_MXDMA_2048 | TCR_CRC | TCR_RETRY_TX;
    outl(iobase + TCR, tcr_value);
    
    // Enable early TX/RX for better performance
    outw(iobase + RX_EARLY_CNT, 0x08);
    
    // Reset all transmit slots
    for (int i = 0; i < 4; i++) {
        outl(iobase + TX_STATUS0 + (i * 4), 0);
        outl(iobase + TX_ADDR0 + (i * 4), 0);
    }
    
    // Configure receiver:
    // - Accept broadcast/multicast/all packets
    // - Use large DMA burst size
    uint32_t rcr_value = RCR_AB | RCR_AM | RCR_APM | RCR_AAP | RCR_MXDMA_1024 | RCR_RXFTH_1;
    outl(iobase + RCR, rcr_value);
    
    // Reset pointers
    outl(iobase + CAPR, 0);
    outl(iobase + 0x44, 0);  // CBR
    
    // Enable Tx/Rx - do this after all configuration
    outb(iobase + CMD, CMD_RX_ENABLE | CMD_TX_ENABLE);
    
    // Enable all useful interrupts
    uint16_t imr_value = INT_ROK | INT_TOK | INT_RXERR | INT_TXERR | 
                        INT_RX_BUFF_OF | INT_LINK_CHG | INT_RX_FIFO_OF;
    outw(iobase + IMR, imr_value);
    
    // Clear any pending interrupts
    outw(iobase + ISR, 0xffff);
    
    rx_offset = 0;
    
    terminal_writestring("[rtl8139] Initialization complete\n");
}

int rtl8139_send(const void* data, size_t len) {
    if (len > 1792) return -1;  // Max MTU
    
    static int tx_slot = 0;
    int timeout;
    
    terminal_writestring("[rtl8139] Sending packet (");
    char lenbuf[8];
    int llen = 0;
    int tlen = len;
    do {
        lenbuf[llen++] = '0' + (tlen % 10);
        tlen /= 10;
    } while (tlen > 0);
    while (llen > 0) terminal_putchar(lenbuf[--llen]);
    terminal_writestring(" bytes) using slot ");
    terminal_putchar('0' + tx_slot);
    terminal_writestring("\n");
    
    // Get initial status
    uint32_t tx_status = inl(iobase + TX_STATUS0 + (tx_slot * 4));
    
    // If slot is busy, try to reset it
    if (tx_status & TX_HOST_OWNS) {
        terminal_writestring("[rtl8139] Resetting busy TX slot\n");
        outl(iobase + TX_STATUS0 + (tx_slot * 4), 0);
        outw(iobase + ISR, INT_TOK); // Clear any pending TX interrupt
        
        // Wait briefly for reset to take effect
        timeout = 100;
        while ((inl(iobase + TX_STATUS0 + (tx_slot * 4)) & TX_HOST_OWNS) && timeout > 0) {
            timeout--;
        }
        
        if (timeout == 0) {
            terminal_writestring("[rtl8139] Failed to reset TX slot, trying next one\n");
            tx_slot = (tx_slot + 1) % 4;
            return rtl8139_send(data, len); // Try again with next slot
        }
    }
    
    // Clear any old status
    outl(iobase + TX_STATUS0 + (tx_slot * 4), 0);
    
    // Allocate and copy to a 32-bit aligned buffer
    uint8_t* tx_buf = (uint8_t*)malloc(len + 4);
    if (!tx_buf) {
        terminal_writestring("[rtl8139] Failed to allocate TX buffer!\n");
        return -1;
    }
    
    // Align buffer and copy data
    uint8_t* aligned_buf = (uint8_t*)(((uintptr_t)tx_buf + 3) & ~3);
    memcpy(aligned_buf, data, len);
    
    // Set up and start transmission
    outl(iobase + TX_ADDR0 + (tx_slot * 4), (uint32_t)aligned_buf);
    outl(iobase + TX_STATUS0 + (tx_slot * 4), (len & 0x1FFF) | TX_HOST_OWNS);
    
    // Wait for completion
    timeout = 1000;
    while (timeout > 0) {
        tx_status = inl(iobase + TX_STATUS0 + (tx_slot * 4));
        
        // Check for completion
        if (!(tx_status & TX_HOST_OWNS)) {
            if (tx_status & TX_STAT_OK) {
                terminal_writestring("[rtl8139] TX completed successfully\n");
                outw(iobase + ISR, INT_TOK);
                free(tx_buf);
                tx_slot = (tx_slot + 1) % 4;
                return len;
            }
            // Check for errors
            if (tx_status & (TX_ABORTED | TX_UNDERRUN | TX_CARRIER_LOST)) {
                if (tx_status & TX_ABORTED) terminal_writestring("[rtl8139] TX aborted!\n");
                if (tx_status & TX_UNDERRUN) terminal_writestring("[rtl8139] TX buffer underrun!\n");
                if (tx_status & TX_CARRIER_LOST) terminal_writestring("[rtl8139] TX lost carrier!\n");
                free(tx_buf);
                tx_slot = (tx_slot + 1) % 4;
                return -1;
            }
        }
        
        timeout--;
    }
    
    terminal_writestring("[rtl8139] TX completion timeout!\n");
    free(tx_buf);
    tx_slot = (tx_slot + 1) % 4;
    return -1;
}

int rtl8139_receive(void* buf, size_t maxlen) {
    // Check if packet is available
    uint16_t status = inw(iobase + ISR);
    
    // Handle receive errors first
    if (status & (INT_RXERR | INT_RX_BUFF_OF | INT_RX_FIFO_OF)) {
        terminal_writestring("[rtl8139] RX error detected: ");
        if (status & INT_RXERR) terminal_writestring("general error ");
        if (status & INT_RX_BUFF_OF) terminal_writestring("buffer overflow ");
        if (status & INT_RX_FIFO_OF) terminal_writestring("FIFO overflow ");
        terminal_writestring("\n");
        
        // Clear error flags
        outw(iobase + ISR, INT_RXERR | INT_RX_BUFF_OF | INT_RX_FIFO_OF);
        
        // Reset receiver
        uint8_t cmd = inb(iobase + CMD);
        outb(iobase + CMD, cmd & ~CMD_RX_ENABLE);  // Disable receiver
        outl(iobase + RX_BUF, (uint32_t)rx_buffer);  // Reset buffer
        rx_offset = 0;
        outb(iobase + CMD, cmd | CMD_RX_ENABLE);  // Re-enable receiver
        return -1;
    }
    
    // Check for new packet
    if (!(status & INT_ROK)) {
        // Only print status if it's not empty and not just a TX OK
        if (status != 0 && status != INT_TOK) {
            terminal_writestring("[rtl8139] No packet available (ISR=");
            char hexbuf[5];
            for (int i = 0; i < 4; i++) {
                int nibble = (status >> (12 - i*4)) & 0xF;
                hexbuf[i] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
            }
            hexbuf[4] = '\0';
            terminal_writestring(hexbuf);
            terminal_writestring(")\n");
        }
        return 0;
    }
    
    // Clear ROK interrupt
    outw(iobase + ISR, INT_ROK);
    
    // Read packet header
    uint16_t rx_status = *(uint16_t*)(rx_buffer + rx_offset);
    uint16_t rx_len = *(uint16_t*)(rx_buffer + rx_offset + 2);
    
    terminal_writestring("[rtl8139] Packet received (");
    char lenbuf[8];
    int i = 0;
    int tlen = rx_len;
    do {
        lenbuf[i++] = '0' + (tlen % 10);
        tlen /= 10;
    } while (tlen > 0);
    while (i > 0) terminal_putchar(lenbuf[--i]);
    terminal_writestring(" bytes)\n");
    
    if (!(rx_status & RX_OK)) {
        terminal_writestring("[rtl8139] Packet error: ");
        if (rx_status & RX_BAD) terminal_writestring("bad packet ");
        if (rx_status & RX_FAE) terminal_writestring("frame alignment error ");
        if (rx_status & RX_CRC) terminal_writestring("CRC error ");
        if (rx_status & RX_LONG) terminal_writestring("too long ");
        if (rx_status & RX_RUNT) terminal_writestring("too short ");
        if (rx_status & RX_INVALID) terminal_writestring("invalid ");
        terminal_writestring("\n");
        return -1;
    }
    
    // Validate packet length
    if (rx_len > maxlen || rx_len < 64) {
        terminal_writestring("[rtl8139] Invalid packet length\n");
        return -1;
    }
    
    // Copy packet to user buffer (skip status and length fields)
    memcpy(buf, rx_buffer + rx_offset + 4, rx_len);
    
    // Update rx_offset
    rx_offset = (rx_offset + rx_len + 4 + 3) & ~3;  // Align to 32-bit boundary
    if (rx_offset > 8192) {
        rx_offset -= 8192;
    }
    
    // Update CAPR - Must be 16 bytes behind
    outw(iobase + CAPR, (rx_offset - 16) & 0xFFFF);
    
    return rx_len;
}
