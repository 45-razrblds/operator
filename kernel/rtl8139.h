#ifndef RTL8139_H
#define RTL8139_H

#include <stdint.h>
#include <stddef.h>

void rtl8139_init(void);
int rtl8139_send(const void* data, size_t len);
int rtl8139_receive(void* buf, size_t maxlen);

#endif
