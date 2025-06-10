#ifndef LWIPOPTS_H
#define LWIPOPTS_H

/* Memory options */
#define MEM_ALIGNMENT           4
#define MEM_SIZE               16000
#define MEMP_NUM_PBUF          16
#define MEMP_NUM_UDP_PCB       4
#define MEMP_NUM_TCP_PCB       4
#define MEMP_NUM_TCP_PCB_LISTEN 8
#define MEMP_NUM_TCP_SEG       16
#define PBUF_POOL_SIZE         16

/* TCP options */
#define LWIP_TCP               1
#define TCP_TTL                255
#define TCP_QUEUE_OOSEQ        0
#define TCP_MSS                1460
#define TCP_SND_BUF            (4 * TCP_MSS)
#define TCP_WND               (2 * TCP_MSS)

/* ICMP options */
#define LWIP_ICMP             1

/* DHCP options */
#define LWIP_DHCP            1

/* IP options */
#define IP_REASSEMBLY        0
#define IP_FRAG              0
#define IP_DEFAULT_TTL       255

/* UDP options */
#define LWIP_UDP            1
#define UDP_TTL            255

/* Statistics options */
#define LWIP_STATS         0
#define LWIP_PROVIDE_ERRNO 1

/* Platform specific locking */
#define NO_SYS            1
#define NO_SYS_NO_TIMERS  1

#define LWIP_NETIF_LINK_CALLBACK 1
#define LWIP_NETIF_STATUS_CALLBACK 1

#endif /* LWIPOPTS_H */
