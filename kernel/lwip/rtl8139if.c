#include "rtl8139if.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ethernet.h"

/* Define those to better describe your network interface */
#define IFNAME0 'e'
#define IFNAME1 'n'

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    uint8_t buf[1792]; // RTL8139 max MTU
    int offset = 0;

    for(q = p; q != NULL; q = q->next) {
        if (offset + q->len > sizeof(buf))
            return ERR_BUF;
        memcpy(buf + offset, q->payload, q->len);
        offset += q->len;
    }

    if (rtl8139_send(buf, offset) < 0)
        return ERR_IF;

    return ERR_OK;
}

static struct pbuf *low_level_input(struct netif *netif)
{
    struct pbuf *p, *q;
    uint16_t len;
    uint8_t buf[1792]; // RTL8139 max MTU

    len = rtl8139_receive(buf, sizeof(buf));
    if (len <= 0)
        return NULL;

    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (p == NULL)
        return NULL;

    for(q = p; q != NULL; q = q->next) {
        memcpy(q->payload, buf + (p->tot_len - q->tot_len), q->len);
    }

    return p;
}

err_t rtl8139if_init(struct netif *netif)
{
    /* Initialize interface hostname */
    netif->hostname = "rtl8139";

    /* Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second. */
    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;

    /* Initialize RTL8139 */
    rtl8139_init();

    return ERR_OK;
}

void rtl8139if_input(struct netif *netif)
{
    struct pbuf *p;

    p = low_level_input(netif);
    if (p != NULL) {
        if (ethernet_input(p, netif) != ERR_OK) {
            pbuf_free(p);
        }
    }
}
