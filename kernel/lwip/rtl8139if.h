#ifndef RTL8139IF_H
#define RTL8139IF_H

#include "../rtl8139.h"
#include "lwip/netif.h"
#include "lwip/err.h"

err_t rtl8139if_init(struct netif *netif);
void rtl8139if_input(struct netif *netif);

#endif /* RTL8139IF_H */
