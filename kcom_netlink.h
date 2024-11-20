#ifndef __KCOM_NETLINK_H__
#define __KCOM_NETLINK_H__

#include <net/sock.h>

struct sock* kcom_netlink_open(int protocol_type, unsigned int group, void (*cb_on_recv)(struct sk_buff*));
void kcom_netlink_close(struct sock* sk);
int kcom_netlink_send(struct sock* sk, int remote_id, const void* data, int data_size);
int kcom_netlink_send_broadcast(struct sock* sk, int group, const void* data, int data_size);

#endif /* __KCOM_NETLINK_H__ */

