#ifndef __KCOM_NETLINK_H__
#define __KCOM_NETLINK_H__

#include <net/sock.h>
#include <net/genetlink.h>

struct sock* kcom_netlink_open(int protocol_type, unsigned int group, void (*cb_on_recv)(struct sk_buff*));
void kcom_netlink_close(struct sock* sk);
int kcom_netlink_send(struct sock* sk, int remote_id, const void* data, int data_size);
int kcom_netlink_send_broadcast(struct sock* sk, int group, const void* data, int data_size);

void* kcom_netlink_generic_open(const char* name, void (*cb_on_recv)(int, const unsigned char*, int));
void kcom_netlink_generic_close(void* handle);
int kcom_netlink_generic_send(void* handle, int remote_id, const void* data, int data_size);
int kcom_netlink_generic_send_broadcast(void* handle, int group, const void* data, int data_size);

#endif /* __KCOM_NETLINK_H__ */

